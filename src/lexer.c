#define _POSIX_C_SOURCE 200809L

#include "lexer.h"
#include "path.h"
#include "execution.h"
#include "redirection.h"
#include "pipeline.h"
#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_HISTORY 3

static char *history[MAX_HISTORY];
static int history_count = 0;

void print_prompt(void)
{
    char hostname[256];
    char cwd[PATH_MAX];

    char *user = getenv("USER");

    if (user == NULL)
        user = "unknown";

    if (gethostname(hostname, sizeof(hostname)) != 0)
        strcpy(hostname, "unknown");

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        strcpy(cwd, "unknown");

    printf("%s@%s:%s> ", user, hostname, cwd);
    fflush(stdout);
}

static void add_history(const char *command)
{
    if (command == NULL || command[0] == '\0')
        return;

    if (history_count < MAX_HISTORY) {
        history[history_count] =
            malloc(strlen(command) + 1);

        if (history[history_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        strcpy(history[history_count], command);
        history_count++;

        return;
    }

    free(history[0]);

    history[0] = history[1];
    history[1] = history[2];

    history[2] = malloc(strlen(command) + 1);

    if (history[2] == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(history[2], command);
}

static void print_history(void)
{
    if (history_count == 0) {
        printf("no valid commands\n");
        return;
    }

    for (int i = 0; i < history_count; i++)
        printf("%s\n", history[i]);
}

static void free_history(void)
{
    for (int i = 0; i < history_count; i++)
        free(history[i]);
}

static bool handle_cd(tokenlist *tokens)
{
    if (tokens->size > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return false;
    }

    const char *path;

    if (tokens->size == 1) {

        path = getenv("HOME");

        if (path == NULL) {
            fprintf(stderr, "cd: HOME is not set\n");
            return false;
        }

    } else {
        path = tokens->items[1];
    }

    if (chdir(path) != 0) {
        perror("cd");
        return false;
    }

    return true;
}

int main(void)
{
    job_table jobs;
    jobs_init(&jobs);

    while (1) {
        print_prompt();

        char *input = get_input();

        if (input == NULL) {
            printf("\n");
            break;
        }

        if (input[0] == '\0') {
            free(input);
            jobs_check(&jobs);
            continue;
        }

        tokenlist *tokens = get_tokens(input);

        expand_environment_variables(tokens);
        expand_tilde(tokens);

        if (tokens->size > 0 &&
            strcmp(tokens->items[0], "exit") == 0) {

            jobs_wait(&jobs);

            print_history();

            free_tokens(tokens);
            free(input);
            free_history();

            return 0;
        }

        if (tokens->size > 0 &&
            strcmp(tokens->items[0], "cd") == 0) {

            if (handle_cd(tokens))
                add_history(input);

            free_tokens(tokens);
            free(input);

            jobs_check(&jobs);
            continue;
        }

        if (tokens->size > 0 &&
            strcmp(tokens->items[0], "jobs") == 0) {

            add_history(input);

            jobs_print(&jobs);

            free_tokens(tokens);
            free(input);

            jobs_check(&jobs);
            continue;
        }

        bool background = extract_background(tokens);

        char *cmdline = join_tokens(tokens);

        pipeline *pl = split_pipeline(tokens);

        redirection_info redir;
        init_redirection(&redir);

        bool valid_command = false;

        if (pl->count == 1) {

            tokenlist *cmd = pl->commands[0];

            if (parse_redirection(cmd, &redir) &&
                cmd->size > 0) {

                if (resolve_command_path(cmd)) {

                    if (background) {
                        execute_pipeline(
                            pl,
                            &redir,
                            true,
                            &jobs,
                            cmdline
                        );

                    } else {
                        execute_external_command(
                            cmd,
                            &redir
                        );
                    }

                    valid_command = true;
                }
            }

        } else {
            bool all_found = true;

            for (size_t i = 0; i < pl->count; i++) {
                if (!resolve_command_path(
                        pl->commands[i])) {

                    all_found = false;
                }
            }

            if (all_found) {

                execute_pipeline(
                    pl,
                    NULL,
                    background,
                    &jobs,
                    cmdline
                );

                valid_command = true;
            }
        }

        if (valid_command)
            add_history(input);

        jobs_check(&jobs);

        free_redirection(&redir);
        free_pipeline(pl);
        free(cmdline);
        free(input);
        free_tokens(tokens);
    }

    jobs_wait(&jobs);
    free_history();

    return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	bool got_line = false;
	while (fgets(line, 5, stdin) != NULL)
	{
		got_line = true;
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}

	if (!got_line && feof(stdin))
		return NULL;

	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    size_t i = tokens->size;

    tokens->items =
        (char **)realloc(tokens->items, (i + 2) * sizeof(char *));

    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;

    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void expand_environment_variables(tokenlist *tokens)
{
    for (size_t i = 0; i < tokens->size; i++) {

        char *token = tokens->items[i];

        if (token[0] != '$')
            continue;

        char *name = token + 1;
        char *value = getenv(name);

        if (value == NULL)
            value = "";

        char *replacement = malloc(strlen(value) + 1);

        if (replacement == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        strcpy(replacement, value);

        free(tokens->items[i]);
        tokens->items[i] = replacement;
    }
}

void free_tokens(tokenlist *tokens)
{
    for (size_t i = 0; i < tokens->size; i++)
        free(tokens->items[i]);

    free(tokens->items);
    free(tokens);
}

void expand_tilde(tokenlist *tokens)
{
    char *home = getenv("HOME");

    if (home == NULL)
        return;

    for (size_t i = 0; i < tokens->size; i++) {
        char *token = tokens->items[i];

        if (strcmp(token, "~") == 0) {
            char *replacement = malloc(strlen(home) + 1);

            if (replacement == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            strcpy(replacement, home);

            free(tokens->items[i]);
            tokens->items[i] = replacement;
        }
        else if (strncmp(token, "~/", 2) == 0) {
            size_t length = strlen(home) + strlen(token + 1) + 1;

            char *replacement = malloc(length);

            if (replacement == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            strcpy(replacement, home);
            strcat(replacement, token + 1);

            free(tokens->items[i]);
            tokens->items[i] = replacement;
        }
    }
}