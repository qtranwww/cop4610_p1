#define _POSIX_C_SOURCE 200809L

#include "pipeline.h"
#include "execution.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

pipeline *split_pipeline(tokenlist *tokens)
{
    pipeline *p = malloc(sizeof(pipeline));

    if (p == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    p->commands = malloc(sizeof(tokenlist *));
    p->count = 0;

    tokenlist *current = new_tokenlist();

    for (size_t i = 0; i < tokens->size; i++) {

        if (strcmp(tokens->items[i], "|") == 0) {
            p->commands =
                realloc(p->commands, (p->count + 1) * sizeof(tokenlist *));
            p->commands[p->count++] = current;

            current = new_tokenlist();
            continue;
        }

        add_token(current, tokens->items[i]);
    }

    p->commands =
        realloc(p->commands, (p->count + 1) * sizeof(tokenlist *));
    p->commands[p->count++] = current;

    return p;
}

void free_pipeline(pipeline *p)
{
    for (size_t i = 0; i < p->count; i++)
        free_tokens(p->commands[i]);

    free(p->commands);
    free(p);
}

bool extract_background(tokenlist *tokens)
{
    if (tokens->size == 0)
        return false;

    size_t last = tokens->size - 1;

    if (strcmp(tokens->items[last], "&") != 0)
        return false;

    free(tokens->items[last]);
    tokens->items[last] = NULL;
    tokens->size--;

    return true;
}

char *join_tokens(tokenlist *tokens)
{
    size_t length = 1;

    for (size_t i = 0; i < tokens->size; i++)
        length += strlen(tokens->items[i]) + 1;

    char *result = malloc(length);

    if (result == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    result[0] = '\0';

    for (size_t i = 0; i < tokens->size; i++) {
        strcat(result, tokens->items[i]);

        if (i + 1 < tokens->size)
            strcat(result, " ");
    }

    return result;
}

static void run_pipeline_stage(
    tokenlist *cmd,
    redirection_info *redir,
    int in_fd,
    int out_fd)
{
    if (in_fd != STDIN_FILENO) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }

    if (out_fd != STDOUT_FILENO) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }

    if (redir != NULL) {
        if (redir->input_file != NULL &&
            setup_input_redirection(redir->input_file) != 0) {
            exit(EXIT_FAILURE);
        }

        if (redir->output_file != NULL &&
            setup_output_redirection(redir->output_file) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    execv(cmd->items[0], cmd->items);

    perror("execv");
    exit(EXIT_FAILURE);
}

void execute_pipeline(
    pipeline *p,
    redirection_info *redir,
    bool background,
    job_table *jobs,
    const char *cmdline)
{
    size_t n = p->count;
    pid_t *pids = malloc(n * sizeof(pid_t));

    if (pids == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int in_fd = STDIN_FILENO;

    for (size_t i = 0; i < n; i++) {

        int fds[2] = { -1, -1 };
        bool has_next = (i + 1 < n);

        if (has_next && pipe(fds) < 0) {
            perror("pipe");
            free(pids);
            return;
        }

        int out_fd = has_next ? fds[1] : STDOUT_FILENO;

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            free(pids);
            return;
        }

        if (pid == 0) {
            if (has_next)
                close(fds[0]);

            run_pipeline_stage(p->commands[i], redir, in_fd, out_fd);
        }

        pids[i] = pid;

        if (in_fd != STDIN_FILENO)
            close(in_fd);

        if (has_next) {
            close(fds[1]);
            in_fd = fds[0];
        }
    }

    if (background) {
        jobs_add(jobs, pids, n, cmdline);
    }
    else {
        for (size_t i = 0; i < n; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    free(pids);
}
