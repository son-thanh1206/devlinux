#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SEARCHER_PATH "./bin/searcher"
#define DATA_FILE     "students.txt"
#define ID_BUF_SIZE   64

extern char **environ;

static const char *describe(int code)
{
    switch (code) {
    case 0:  return "Found";
    case 1:  return "Not found";
    case 2:  return "Error (file / argument / execve)";
    default: return "Unknown exit code";
    }
}

int main(void)
{
    printf("\n=============================================\n");
    printf("   STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("   (fork + execve | file: %s)\n", DATA_FILE);
    printf("=============================================\n");
    printf("[MANAGER] PID: %d\n", (int)getpid());
    printf("Enter student ID ('quit' to exit).\n");

    char id[ID_BUF_SIZE];

    while (1) {
        printf("\n---------------------------------------------\n");
        printf("Student ID: ");
        fflush(stdout);

        if (fgets(id, sizeof id, stdin) == NULL) {
            printf("\n[MANAGER] End of input. Goodbye!\n");
            break;
        }

        id[strcspn(id, "\r\n")] = '\0';
        if (id[0] == '\0')
            continue;

        if (strcmp(id, "quit") == 0) {
            printf("[MANAGER] Exiting. Goodbye!\n");
            break;
        }

        fflush(stdout);

        pid_t pid = fork();

        if (pid < 0) {
            perror("[MANAGER] fork");
            continue;
        }

        if (pid == 0) {
            char *args[] = { SEARCHER_PATH, id, DATA_FILE, NULL };
            execve(SEARCHER_PATH, args, environ);
            /* Only reached if execve() fails — on success it never returns */
            perror("execve failed");
            exit(2);
        }

        printf("\n[MANAGER] fork() → child PID: %d\n", (int)pid);
        printf("[MANAGER] Waiting for child (waitpid)...\n\n");
        fflush(stdout);

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("[MANAGER] waitpid");
            continue;
        }

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("\n[MANAGER] Child (PID %d) exited. code=%d → %s\n",
                   (int)pid, code, describe(code));
        } else if (WIFSIGNALED(status)) {
            printf("\n[MANAGER] Child (PID %d) killed by signal %d\n",
                   (int)pid, WTERMSIG(status));
        }
    }

    return 0;
}