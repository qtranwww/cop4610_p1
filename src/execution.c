#define _POSIX_C_SOURCE 200809L

#include "execution.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


int setup_input_redirection(const char *filename)
{
    struct stat info;

    if (stat(filename, &info) != 0) {
        perror(filename);
        return -1;
    }

    if (!S_ISREG(info.st_mode)) {
        fprintf(stderr,
                "%s: not a regular file\n",
                filename);
        return -1;
    }

    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        perror(filename);
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}


int setup_output_redirection(const char *filename)
{
    int fd = open(
        filename,
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR
    );

    if (fd < 0) {
        perror(filename);
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}


void execute_external_command(
    tokenlist *tokens,
    redirection_info *redir)
{
    if (tokens == NULL || tokens->size == 0)
        return;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {

        if (redir->input_file != NULL) {
            if (setup_input_redirection(
                    redir->input_file) != 0) {

                exit(EXIT_FAILURE);
            }
        }

        if (redir->output_file != NULL) {
            if (setup_output_redirection(
                    redir->output_file) != 0) {

                exit(EXIT_FAILURE);
            }
        }

        execv(tokens->items[0], tokens->items);

        /*
         * Only reached if execv failed.
         */
        perror("execv");
        exit(EXIT_FAILURE);
    }

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
    }
}