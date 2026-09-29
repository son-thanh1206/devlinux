// Exit code	Meaning
// 0	Student found — print full record
// 1	Student not found — print message
// 2	File or argument error — perror()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LINE_BUF_SIZE 256
 
static const char *classify(double gpa)
{
    if (gpa >= 8.5) return "Excellent";
    if (gpa >= 7.0) return "Good";
    if (gpa >= 5.0) return "Average";
    return "Poor";
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <student_id> <data_file>\n", argv[0]);
        exit(2);
    }
    
    const char *student_id = argv[1];
    const char *file_path = argv[2];

    printf("[SEARCHER] PID: %d | PPID: %d\n", (int)getpid(), (int)getppid());
    printf("[SEARCHER] Searching for \"%s\" in %s...\n", student_id, file_path);

    FILE *fp = fopen(file_path, "r");
    if(fp == NULL)
    {
        perror("[SEARCHER] fopen");
        exit(2);
    }

    char line[LINE_BUF_SIZE];

     while (fgets(line, sizeof line, fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
 
        char *id      = strtok(line, "|");
        char *name    = strtok(NULL, "|");
        char *cls     = strtok(NULL, "|");
        char *gpa_str = strtok(NULL, "|");
 
        if (!id || !name || !cls || !gpa_str)
            continue;
 
        if (strcmp(id, student_id) != 0)
            continue;
 
        char *end;
        double gpa = strtod(gpa_str, &end);
        if (end == gpa_str) {
            fprintf(stderr, "[SEARCHER] Invalid GPA for %s: \"%s\"\n", id, gpa_str);
            fclose(fp);
            exit(2);
        }
 
        printf("\n========== SEARCH RESULT ==========\n");
        printf("  ID      : %s\n", id);
        printf("  Name    : %s\n", name);
        printf("  Class   : %s\n", cls);
        printf("  GPA     : %s\n", gpa_str);
        printf("  Grade   : %s\n", classify(gpa));
        printf("====================================\n");
 
        fclose(fp);
        exit(0);
    }

    if (ferror(fp)) {
        perror("[SEARCHER] fgets");
        fclose(fp);
        exit(2);
    }
 
    fclose(fp);
    printf("[SEARCHER] No student found with ID: %s\n", student_id);
    exit(1);

    return 0;
}