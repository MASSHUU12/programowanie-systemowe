#include "timer.h"
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define TTL_MIN 1

typedef struct {
  pthread_t tid;
  int32_t ttl;
  int32_t thread_num;
  volatile bool running;
} thread_data_t;

thread_data_t *thread_array = NULL;
int32_t num_threads = 0;

uint64_t calculate_factorial(const uint16_t n) {
  uint64_t result = 1;
  for (uint16_t i = 2; i <= n; i++) {
    result *= i;
  }
  return result;
}

void signal_handler(const int32_t signo) {
  printf("[THREAD %lu] Received signal %d. Execution time: %.2f ms\n",
         pthread_self(), signo, stop());
  pthread_exit(NULL);
}

void *thread_function(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  sigaction(SIGUSR1, &sa, NULL);

  start();

  printf("[THREAD %lu] Thread %d started\n", pthread_self(), data->thread_num);

  data->running = true;

  uint64_t factorial = 1;
  uint16_t n = 1;
  uint32_t counter = 0;

  while (data->running) {
    factorial = calculate_factorial(n);

    if (counter % 10 == 0) {
      printf("[THREAD %d] Calculated %u! = %lu\n", data->thread_num, n,
             factorial);
    }

    n = (n % 20) + 1; // Reset n to avoid overflow
    counter++;
    usleep(100000); // Sleep a bit to not consume 100% CPU
  }

  return NULL;
}

int32_t random_number(int32_t min_num, int32_t max_num) {
  if (min_num > max_num) {
    min_num ^= max_num;
    max_num ^= min_num;
    min_num ^= max_num;
  }
  return (rand() % (max_num - min_num + 1)) + min_num;
}

int main(int argc, char **argv) {
  srand(time(NULL));

  int32_t opt, threads = 4, ttl_max = 5;

  while ((opt = getopt(argc, argv, "m:t:")) != -1) {
    switch (opt) {
    case 't':
      if (optarg) {
        threads = atoi(optarg);
        if (threads <= 0) {
          fprintf(stderr, "Error: -t must be a positive integer.\n");
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "Error: -t option requires an argument.\n");
        return EXIT_FAILURE;
      }
      break;
    case 'm':
      if (optarg) {
        ttl_max = atoi(optarg);
        if (ttl_max <= 0) {
          fprintf(stderr, "Error: -m must be a positive integer.\n");
          return EXIT_FAILURE;
        }
      } else {
        fprintf(stderr, "Error: -m option requires an argument.\n");
        return EXIT_FAILURE;
      }
      break;
    default:
      fprintf(stderr, "Usage: %s [ -t threads ] [ -m max_ttl ]\n", argv[0]);
      return EXIT_FAILURE;
    }
  }

  printf("Creating %d threads with maximum TTL of %d seconds\n", threads,
         ttl_max);

  thread_array = calloc(threads, sizeof(thread_data_t));
  if (!thread_array) {
    fprintf(stderr, "Memory allocation failed\n");
    return EXIT_FAILURE;
  }
  num_threads = threads;

  for (int32_t i = 0; i < threads; i++) {
    thread_array[i].ttl = random_number(TTL_MIN, ttl_max);
    thread_array[i].thread_num = i + 1;
    thread_array[i].running = false;

    if (pthread_create(&thread_array[i].tid, NULL, thread_function,
                       &thread_array[i]) != 0) {
      fprintf(stderr, "Error creating thread %d\n", i + 1);
      return EXIT_FAILURE;
    }

    printf("[PARENT] Thread %d (ID: %lu) created with TTL: %d seconds\n", i + 1,
           thread_array[i].tid, thread_array[i].ttl);
  }

  struct timeval start_tv;
  gettimeofday(&start_tv, NULL);

  bool *thread_signaled = calloc(threads, sizeof(bool));
  if (!thread_signaled) {
    fprintf(stderr, "Memory allocation failed\n");
    free(thread_array);
    return EXIT_FAILURE;
  }

  bool all_signaled = false;
  while (!all_signaled) {
    struct timeval current_tv;
    gettimeofday(&current_tv, NULL);

    double elapsed = (current_tv.tv_sec - start_tv.tv_sec) +
                   (current_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

    all_signaled = true;

    for (int32_t i = 0; i < threads; i++) {
      if (!thread_signaled[i] && elapsed >= thread_array[i].ttl) {
        printf("[PARENT] Sending signal to Thread %d (ID: %lu) after %.2f "
               "seconds\n",
               i + 1, thread_array[i].tid, elapsed);
        thread_array[i].running = false;
        pthread_kill(thread_array[i].tid, SIGUSR1);
        thread_signaled[i] = true;
      }

      if (!thread_signaled[i]) {
        all_signaled = false;
      }
    }

    usleep(50000); // 50ms
  }

  for (int32_t i = 0; i < threads; i++) {
    pthread_join(thread_array[i].tid, NULL);
    printf("[PARENT] Thread %d (ID: %lu) joined\n", i + 1, thread_array[i].tid);
  }

  free(thread_signaled);
  free(thread_array);

  return EXIT_SUCCESS;
}
