#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t reading_count = 0;

int random_number()
{
    return rand() % 100; // Generate a random number between 0 and 99
}

/* Async-signal-safe: write a string literal to stdout. */
#define WRITE_STR(s) write(STDOUT_FILENO, (s), sizeof(s) - 1)

/* Async-signal-safe: write a non-negative int in decimal to stdout. */
static void write_uint(unsigned int n)
{
    char buf[16];
    int i = (int)sizeof(buf);

    do {
        buf[--i] = (char)('0' + n % 10);
        n /= 10;
    } while (n > 0);

    write(STDOUT_FILENO, &buf[i], sizeof(buf) - (size_t)i);
}

void handle_sigint(int sig)
{
    (void)sig;
    WRITE_STR("[WARN] Received SIGINT, ignoring...\n");
}
void handle_sigterm(int sig) {
  (void)sig;
  WRITE_STR("[INFO] Received SIGTERM, shutting down gracefully...\n");
  _exit(0);
}
void handle_sigusr1(int sig) {
  (void)sig;
  WRITE_STR("[REPORT] Total readings so far: ");
  write_uint((unsigned int)reading_count);
  WRITE_STR("\n");
}

int main() {
  // Register signal handlers
  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigterm);
  signal(SIGUSR1, handle_sigusr1);

  printf("[INFO] Main process running...(pid = %d)\n", getpid());

  while (1) {
    int num = random_number();
    reading_count++;
    printf("[INFO] Sensor reading #%d: temperature=%d\n", reading_count, num);
    sleep(1);
  }
}