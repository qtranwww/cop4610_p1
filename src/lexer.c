#define _POSIX_C_SOURCE 200809L

#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    tokenlist *tokens = new_tokenlist();
    size_t i = 0;

    while (input[i] != '\0') {
        while (input[i] == ' ' || input[i] == '\t' || input[i] == '\n')
            i++;
        if (input[i] == '\0')
            break;

        if (input[i] == '|' || input[i] == '<' || input[i] == '>') {
            char operator[2] = { input[i], '\0' };
            add_token(tokens, operator);
            i++;
            continue;
        }

        size_t start = i;
        while (input[i] != '\0' && input[i] != ' ' && input[i] != '\t' &&
               input[i] != '\n' && input[i] != '|' && input[i] != '<' &&
               input[i] != '>')
            i++;

        size_t length = i - start;
        char *word = malloc(length + 1);
        if (word == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        memcpy(word, input + start, length);
        word[length] = '\0';
        add_token(tokens, word);
        free(word);
    }

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
