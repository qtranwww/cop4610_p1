#pragma once

#include "lexer.h"
#include "redirection.h"

int setup_input_redirection(const char *filename);
int setup_output_redirection(const char *filename);

void execute_external_command(
    tokenlist *tokens,
    redirection_info *redir
);