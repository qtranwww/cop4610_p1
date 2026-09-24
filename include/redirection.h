#pragma once

#include <stdbool.h>
#include "lexer.h"

typedef struct {
    char *input_file;
    char *output_file;
} redirection_info;

void init_redirection(redirection_info *redir);
bool parse_redirection(tokenlist *tokens, redirection_info *redir);
void free_redirection(redirection_info *redir);