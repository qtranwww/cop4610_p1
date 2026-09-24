#define _POSIX_C_SOURCE 200809L

#include "execution.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void execute_external_command(tokenlist *tokens)
{
    if (tokens == NULL || tokens->size == 0)
        return;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execv(tokens->items[0], tokens->items);

        /*
         * execv only returns if execution failed.
         */
        perror("execv");
        exit(EXIT_FAILURE);
    }

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
    }
}