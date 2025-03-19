#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
      printf("Child process: PID=%d\n", getpid());
    }
    else if(pid > 0) {
      printf("Parent process: PID=%d\n", getpid());
      wait(NULL);
    }
    else {
      printf("Fork failed!\n");
      return 1;
    }

    return 0;
}
