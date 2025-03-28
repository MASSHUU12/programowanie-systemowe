#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

double timespec_diff_sec(const struct timespec *start,
                         const struct timespec *end) {
  return (end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec) / 1e9;
}

int redirect_to_dev_null(const int fd) {
  int dev_null = open("/dev/null", O_WRONLY);
  if (dev_null == -1) {
    perror("open /dev/null");
    return -1;
  }

  if (dup2(dev_null, fd) == -1) {
    perror("dup2");
    close(dev_null);
    return -1;
  }

  close(dev_null);
  return 0;
}

void print_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s [ -v ] [ -c count ] <program> [args ...]\n",
          program_name);
}

int run_command(char **argv, const int verbose, double *real_time,
                double *user_time, double *sys_time) {
  struct timespec start, end;
  struct rusage usage;

  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime");
    return -1;
  }

  const pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    if (!verbose) {
      if (redirect_to_dev_null(STDOUT_FILENO) == -1 ||
          redirect_to_dev_null(STDERR_FILENO) == -1) {
        exit(EXIT_FAILURE);
      }
    }
    execvp(argv[0], argv);
    perror("execvp");
    exit(EXIT_FAILURE);
  }

  if (wait4(pid, NULL, 0, &usage) == -1) {
    perror("wait4");
    return -1;
  }

  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
    perror("clock_gettime");
    return -1;
  }

  *real_time = timespec_diff_sec(&start, &end);
  *user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
  *sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

  return 0;
}

int main(int argc, char **argv) {
  int opt, verbose = 0, count = 1;

  while ((opt = getopt(argc, argv, "vc:")) != -1) {
    switch (opt) {
    case 'v':
      verbose = 1;
      break;
    case 'c':
      if (optarg) {
        count = atoi(optarg);
        if (count <= 0) {
          fprintf(stderr, "Error: -c must be a positive integer.\n");
          return EXIT_FAILURE;
        }
      }
      break;
    default:
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (optind >= argc) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  double total_real = 0.0, total_user = 0.0, total_sys = 0.0;

  for (int i = 0; i < count; ++i) {
    double real_time = 0.0, user_time = 0.0, sys_time = 0.0;

    if (run_command(&argv[optind], verbose, &real_time, &user_time,
                    &sys_time) != 0) {
      return EXIT_FAILURE;
    }

    printf("[Run %d] real: %.3fs, user: %.3fs, sys: %.3fs\n", i + 1, real_time,
           user_time, sys_time);

    total_real += real_time;
    total_user += user_time;
    total_sys += sys_time;
  }

  if (count > 1) {
    printf("[Average] real: %.3fs, user: %.3fs, sys: %.3fs\n",
           total_real / count, total_user / count, total_sys / count);
  }

  return EXIT_SUCCESS;
}
