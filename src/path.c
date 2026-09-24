#define _POSIX_C_SOURCE 200809L

#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


bool resolve_command_path(tokenlist *tokens)
{
    if (tokens == NULL || tokens->size == 0)
        return false;

    char *command = tokens->items[0];

    /*
     * If the command already contains '/', it is already a path.
     * Examples:
     *   ./program
     *   ../program
     *   /usr/bin/ls
     */
    if (strchr(command, '/') != NULL) {
        if (access(command, F_OK) == 0)
            return true;

        fprintf(stderr, "%s: command not found\n", command);
        return false;
    }

    char *path_env = getenv("PATH");

    if (path_env == NULL) {
        fprintf(stderr, "%s: command not found\n", command);
        return false;
    }

    /*
     * strtok() modifies its input, so make a deep copy of PATH.
     */
    char *path_copy = malloc(strlen(path_env) + 1);

    if (path_copy == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(path_copy, path_env);

    char *directory = strtok(path_copy, ":");

    while (directory != NULL) {

        /*
         * +2:
         * one byte for '/'
         * one byte for '\0'
         */
        size_t length =
            strlen(directory) + strlen(command) + 2;

        char *candidate = malloc(length);

        if (candidate == NULL) {
            free(path_copy);
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        snprintf(candidate,
                 length,
                 "%s/%s",
                 directory,
                 command);

        if (access(candidate, F_OK) == 0) {
            free(tokens->items[0]);
            tokens->items[0] = candidate;

            free(path_copy);
            return true;
        }

        free(candidate);

        directory = strtok(NULL, ":");
    }

    free(path_copy);

    fprintf(stderr, "%s: command not found\n", command);

    return false;
}