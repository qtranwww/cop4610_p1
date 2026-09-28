#pragma once

#include "lexer.h"
#include "redirection.h"

#include <stdbool.h>

/* Exit status a child uses when its redirection or exec setup failed. */
#define EXIT_SETUP_FAILED 125

/* True if a child status means the command never got to run. */
bool setup_failed(int status);

int setup_input_redirection(const char *filename);
int setup_output_redirection(const char *filename);

bool execute_external_command(
    tokenlist *tokens,
    redirection_info *redir
);