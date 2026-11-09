#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define WRITE_STR(s) write(STDOUT_FILENO, (s), sizeof(s) - 1)
#define EXIT_CODE 7

static void write_uint(unsigned int n)
{
  char buf[16];
  int i = (int)sizeof(buf);

  do
  {
    buf[--i] = (char)('0' + n % 10);
    n /= 10;
  } while (n > 0);

  write(STDOUT_FILENO, &buf[i], sizeof(buf) - (size_t)i);
}

void handle_sigusr1(int sig)
{
  (void)sig;
  WRITE_STR("[GATEWAY] Worker reported READY signal received\n");
}

int main()
{
  sigset_t block_set;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGUSR1);
  signal(SIGUSR1, handle_sigusr1);

  pid_t pid = fork();

  if (pid < 0)
  {
    perror("fork");
    return 1;
  }

  if (pid == 0)
  {
    sleep(2);                 // initiate worker
    kill(getppid(), SIGUSR1); // send signal to parent
    WRITE_STR("[WORKER] Sent READY signal to gateway\n");
    _exit(EXIT_CODE);
  }

  WRITE_STR("[GATEWAY] Worker PID = ");
  write_uint((unsigned int)pid);
  WRITE_STR("\n");

  sigprocmask(SIG_BLOCK, &block_set, NULL);
  sleep(5); // Wait for 5 seconds before starting the readings
  sigprocmask(SIG_UNBLOCK, &block_set, NULL);

  int status;
  if (waitpid(pid, &status, 0) == -1)
  {
    perror("waitpid");
    return 1;
  }

  if (WIFEXITED(status))
  {
    WRITE_STR("[GATEWAY] Worker exited with code ");
    write_uint((unsigned int)WEXITSTATUS(status));
    WRITE_STR("\n");  
  }
  
  return 0;
}
