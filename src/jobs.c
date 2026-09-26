#define _POSIX_C_SOURCE 200809L

#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void jobs_init(job_table *table)
{
    table->count = 0;
    table->next_id = 1;
}

void jobs_add(
    job_table *table,
    pid_t *pids,
    size_t pid_count,
    const char *cmdline)
{
    if (table->count >= MAX_JOBS) {
        fprintf(stderr, "jobs: too many background processes\n");
        return;
    }

    job *j = malloc(sizeof(job));

    if (j == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    j->id = table->next_id++;
    j->pid_count = pid_count;

    j->pids = malloc(pid_count * sizeof(pid_t));
    j->reaped = malloc(pid_count * sizeof(bool));

    if (j->pids == NULL || j->reaped == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < pid_count; i++) {
        j->pids[i] = pids[i];
        j->reaped[i] = false;
    }

    j->cmdline = malloc(strlen(cmdline) + 1);

    if (j->cmdline == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(j->cmdline, cmdline);

    table->slots[table->count++] = j;

    printf("[%d] %d\n", j->id, j->pids[j->pid_count - 1]);
    fflush(stdout);
}

static void free_job(job *j)
{
    free(j->pids);
    free(j->reaped);
    free(j->cmdline);
    free(j);
}

void jobs_check(job_table *table)
{
    for (int i = 0; i < table->count; i++) {

        job *j = table->slots[i];
        bool all_done = true;

        for (size_t k = 0; k < j->pid_count; k++) {

            if (j->reaped[k])
                continue;

            int status;
            pid_t result = waitpid(j->pids[k], &status, WNOHANG);

            if (result == j->pids[k])
                j->reaped[k] = true;
            else
                all_done = false;
        }

        if (all_done) {
            printf("[%d] + done %s\n", j->id, j->cmdline);
            fflush(stdout);

            free_job(j);

            for (int m = i; m < table->count - 1; m++)
                table->slots[m] = table->slots[m + 1];

            table->count--;
            i--;
        }
    }
}

void jobs_print(job_table *table)
{
    if (table->count == 0) {
        printf("no active background jobs\n");
        return;
    }

    for (int i = 0; i < table->count; i++) {

        job *j = table->slots[i];

        printf("[%d]+ %d %s\n",
               j->id,
               j->pids[j->pid_count - 1],
               j->cmdline);
    }

    fflush(stdout);
}

void jobs_wait(job_table *table)
{
    while (table->count > 0) {

        job *j = table->slots[0];

        for (size_t k = 0; k < j->pid_count; k++) {

            if (j->reaped[k])
                continue;

            int status;

            if (waitpid(
                    j->pids[k], &status, 0) < 0)
            {

                perror("waitpid");
            }

            j->reaped[k] = true;
        }

        free(j->pids);
        free(j->reaped);
        free(j->cmdline);
        free(j);

        for (int i = 0; i < table->count - 1; i++)
            table->slots[i] = table->slots[i + 1];

        table->count--;
    }
}