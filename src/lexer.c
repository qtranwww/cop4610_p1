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

        tokenlist *tokens = get_tokens(input);
		expand_environment_variables(tokens);
		expand_tilde(tokens);

        bool background = extract_background(tokens);
        char *cmdline = join_tokens(tokens);

        pipeline *pl = split_pipeline(tokens);

        redirection_info redir;
        init_redirection(&redir);

        if (pl->count == 1) {
            tokenlist *cmd = pl->commands[0];

            if (parse_redirection(cmd, &redir) && cmd->size > 0) {
                if (resolve_command_path(cmd)) {
                    if (background)
                        execute_pipeline(pl, &redir, true, &jobs, cmdline);
                    else
                        execute_external_command(cmd, &redir);
                }
            }
        }
        else {
            bool all_found = true;

            for (size_t i = 0; i < pl->count; i++) {
                if (!resolve_command_path(pl->commands[i]))
                    all_found = false;
            }

            if (all_found)
                execute_pipeline(pl, NULL, background, &jobs, cmdline);
        }

        jobs_check(&jobs);

        free_redirection(&redir);
        free_pipeline(pl);
        free(cmdline);
        free(input);
        free_tokens(tokens);
    }

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