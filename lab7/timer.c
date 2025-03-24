#include "timer.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

static pthread_key_t time_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static void make_key(void) { pthread_key_create(&time_key, free); }

void start(void) {
  pthread_once(&key_once, make_key);

  struct timeval *start_time = malloc(sizeof(struct timeval));
  if (start_time == NULL)
    return;

  gettimeofday(start_time, NULL);

  pthread_setspecific(time_key, start_time);
}

double stop(void) {
  struct timeval end_time, *start_time;

  gettimeofday(&end_time, NULL);

  start_time = pthread_getspecific(time_key);
  if (start_time == NULL)
    return 0.0;

  double elapsed =
      (end_time.tv_sec - start_time->tv_sec) * 1000.0; // seconds to ms
  elapsed +=
      (end_time.tv_usec - start_time->tv_usec) / 1000.0; // microseconds to ms

  return elapsed;
}
