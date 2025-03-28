#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t stop = 0;
volatile sig_atomic_t child_count = 0;

char *get_current_time_str() {
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  static char timestr[26];
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);
  return timestr;
}

void sigint_handler(int sig) { stop = 1; }

void sigchld_handler(int sig, siginfo_t *info, void *context) {
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    int exit_code = 0;
    if (WIFEXITED(status)) {
      exit_code = WEXITSTATUS(status);
    }
    printf("Child terminated: [ PID: %d ] [ Exit code: %d ] [ %s ]\n", pid,
           exit_code, get_current_time_str());
    child_count--;
  }
}

void sigalrm_handler(int sig) {
  _exit(0); // Will be replaced with actual duration in child process
}

int32_t random_number(int32_t min_num, int32_t max_num) {
  if (min_num > max_num) {
    min_num ^= max_num;
    max_num ^= min_num;
    min_num ^= max_num;
  }
  return (rand() % (max_num - min_num + 1)) + min_num;
}

void setup_signal_handler(int signum, void (*handler)(int), int flags) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handler;
  sa.sa_flags = flags;
  sigemptyset(&sa.sa_mask);
  if (sigaction(signum, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
}

void setup_sigchld_handler() {
  struct sigaction sa_chld;
  memset(&sa_chld, 0, sizeof(sa_chld));
  sa_chld.sa_sigaction = sigchld_handler;
  sa_chld.sa_flags = SA_SIGINFO | SA_RESTART;
  sigemptyset(&sa_chld.sa_mask);
  if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
    perror("sigaction(SIGCHLD)");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {
  int opt, max_lifetime, wait_interval;

  srand(time(NULL));

  while ((opt = getopt(argc, argv, "w:m:")) != -1) {
    switch (opt) {
    case 'w':
      if (optarg) {
        wait_interval = atoi(optarg);
        if (wait_interval <= 0) {
          fprintf(stderr, "Error: -w must be a positive integer.\n");
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "Error: -w option requires an argument.\n");
        return EXIT_FAILURE;
      }
      break;
    case 'm':
      if (optarg) {
        max_lifetime = atoi(optarg);
        if (max_lifetime <= 0) {
          fprintf(stderr, "Error: -m must be a positive integer.\n");
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "Error: -m option requires an argument.\n");
        return EXIT_FAILURE;
      }
      break;
    default:
      fprintf(stderr, "Usage: %s [ -w wait_interval ] [ -m max_lifetime ]\n",
              argv[0]);
      return EXIT_FAILURE;
    }
  }

  setup_sigchld_handler();
  setup_signal_handler(SIGINT, sigint_handler, SA_RESTART);

  printf("Main process started with PID: %d\n", getpid());
  printf("Press Ctrl+C to stop creating new children and wait for existing "
         "ones\n\n");

  while (!stop) {
    const pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      break;
    }

    if (pid == 0) {
      srand(time(NULL) ^ getpid());

      setup_signal_handler(SIGINT, SIG_IGN, 0);
      setup_signal_handler(SIGALRM, sigalrm_handler, SA_RESTART);

      const int duration = random_number(1, max_lifetime);

      printf("[ PID: %d ] [ Duration: %d ] [ %s ]\n", getpid(), duration,
             get_current_time_str());
      fflush(stdout);

      alarm(duration);

      unsigned long long factorial = 1;
      unsigned int i = 1;
      while (1) {
        factorial *= i;
        i++;
        if (i > 20) {
          factorial = 1;
          i = 1;
        }
      }

      _exit(duration);
    } else {
      child_count++;
      sleep(wait_interval);
    }
  }

  printf("\nReceived SIGINT. Waiting for %d children to terminate...\n",
         child_count);

  while (child_count > 0) {
    sleep(1);
    printf("Remaining children: %d\n", child_count);
  }

  printf("All children terminated. Exiting.\n");
  return EXIT_SUCCESS;
}
