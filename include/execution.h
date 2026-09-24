#pragma once

#include "lexer.h"
#include "redirection.h"

void execute_external_command(
    tokenlist *tokens,
    redirection_info *redir
);