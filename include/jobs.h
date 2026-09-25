#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define MAX_JOBS 10

typedef struct {
    int id;
    pid_t *pids;
    bool *reaped;
    size_t pid_count;
    char *cmdline;
} job;

typedef struct {
    job *slots[MAX_JOBS];
    int count;
    int next_id;
} job_table;

void jobs_init(job_table *table);
void jobs_add(job_table *table, pid_t *pids, size_t pid_count, const char *cmdline);
void jobs_check(job_table *table);
