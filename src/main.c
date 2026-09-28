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

        if (tokens->size == 0) {
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
        bool all_found = true;

        for (size_t i = 0; i < pl->count; i++) {
            if (!parse_redirection(pl->commands[i], &redir)) {
                all_found = false;
                continue;
            }

            if (pl->commands[i]->size == 0) {
                fprintf(stderr, pl->count > 1 ?
                        "syntax error: missing command near '|'\n" :
                        "syntax error: missing command\n");
                all_found = false;
                continue;
            }

            if (!resolve_command_path(pl->commands[i]))
                all_found = false;
        }

        if (all_found) {
            if (pl->count == 1 && !background)
                valid_command =
                    execute_external_command(pl->commands[0], &redir);
            else
                valid_command = execute_pipeline(
                    pl, &redir, background, &jobs, cmdline);
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
