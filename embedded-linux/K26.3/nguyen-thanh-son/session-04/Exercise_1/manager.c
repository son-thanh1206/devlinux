#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOTAL_ORDERS 3
typedef struct {
    int   id;
    char  name[50];
    int   quantity;
    float unit_price;
} Order;

void process_order(Order o) {
    float total = o.quantity * o.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n", o.id, o.name, o.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);
    sleep(2);
}

int main()
{
    Order orders[TOTAL_ORDERS] = {
    {1, "Backpack", 2, 350000},
    {2, "Shoes",    1, 500000},
    {3, "Hat",      3, 120000}};

    pid_t pids[TOTAL_ORDERS] = {0};
    int success_count = 0;
    long long total_revenue = 0;

    printf("===================================================\n");
    printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning %d child processes...\n\n", getpid(), TOTAL_ORDERS);

    for(int i = 0; i < TOTAL_ORDERS; i++)
    {
        fflush(stdout);
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            pids[i] = -1;
            continue;
        }

        if (pid == 0)
        {
            // Child process
            process_order(orders[i]);
            exit(EXIT_SUCCESS);
        }
        else 
        {
            // Parent process
            pids[i] = pid;
            printf("[MANAGER] fork() order #%d -> child PID: %d\n", orders[i].id, pid);
        }
    }

    printf("[MANAGER] All %d children spawned. Starting waitpid()...\n\n", TOTAL_ORDERS);
    fflush(stdout);

    for(int i = 0; i < TOTAL_ORDERS; i++)
    {
        int status;

         if (pids[i] < 0) {
            printf("[MANAGER] order #%d: fork() failed → FAILED\n", orders[i].id);
            continue;
        }

        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            continue;
        }

        
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("[MANAGER] waitpid(%d) — order #%d: exit code=%d → %s\n", pids[i], orders[i].id, code, code == 0 ? "SUCCESS" : "FAILED");
            if (code == 0) {
                success_count++;
                total_revenue += (long long)(orders[i].quantity * orders[i].unit_price);
            }
        }
        else 
        {
            printf("[MANAGER] waitpid(%d) — order #%d: terminated abnormally → FAILED\n", pids[i], orders[i].id);
        }
    }

    printf("================= SUMMARY =================.\n");
    printf("  Total orders    : %d\n", TOTAL_ORDERS);
    printf("  Successful      : %d\n", success_count);
    printf("  Failed          : %d\n", TOTAL_ORDERS - success_count);
    printf("  Total Revenue   : %lld VND\n", total_revenue);
    printf("==========================================\n");
    return 0;
}