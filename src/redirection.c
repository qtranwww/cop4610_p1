#define _POSIX_C_SOURCE 200809L

#include "redirection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *source)
{
    char *copy = malloc(strlen(source) + 1);

    if (copy == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, source);

    return copy;
}

void init_redirection(redirection_info *redir)
{
    redir->input_file = NULL;
    redir->output_file = NULL;
}

bool parse_redirection(tokenlist *tokens, redirection_info *redir)
{
    char **new_items =
        malloc((tokens->size + 1) * sizeof(char *));

    if (new_items == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t new_size = 0;

    for (size_t i = 0; i < tokens->size; i++) {

        if (strcmp(tokens->items[i], "<") == 0 ||
            strcmp(tokens->items[i], ">") == 0) {

            if (i + 1 >= tokens->size) {
                fprintf(stderr,
                        "redirection: missing filename\n");

                free(new_items);
                return false;
            }

            char *filename = tokens->items[i + 1];

            if (strcmp(tokens->items[i], "<") == 0) {
                free(redir->input_file);
                redir->input_file = copy_string(filename);
            }
            else {
                free(redir->output_file);
                redir->output_file = copy_string(filename);
            }

            free(tokens->items[i]);
            free(tokens->items[i + 1]);

            i++;

            continue;
        }

        new_items[new_size++] = tokens->items[i];
    }

    new_items[new_size] = NULL;

    free(tokens->items);

    tokens->items = new_items;
    tokens->size = new_size;

    return true;
}

void free_redirection(redirection_info *redir)
{
    free(redir->input_file);
    free(redir->output_file);

    redir->input_file = NULL;
    redir->output_file = NULL;
}