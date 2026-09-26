#pragma once

#include "lexer.h"
#include "redirection.h"
#include "jobs.h"

#include <stdbool.h>

typedef struct {
    tokenlist **commands;
    size_t count;
} pipeline;

pipeline *split_pipeline(tokenlist *tokens);
void free_pipeline(pipeline *p);

bool extract_background(tokenlist *tokens);
char *join_tokens(tokenlist *tokens);

void execute_pipeline(
    pipeline *p,
    redirection_info *redir,
    bool background,
    job_table *jobs,
    const char *cmdline
);
