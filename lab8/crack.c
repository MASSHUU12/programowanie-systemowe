#include <crypt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PASSWORD_LEN 256

typedef struct {
  char *hash;
  char *salt;
  FILE *dictionary;
  int thread_num;
  int total_threads;
} thread_data_t;

pthread_mutex_t dict_mutex = PTHREAD_MUTEX_INITIALIZER;
int found = 0;

void *crack_password(void *arg) {
  const thread_data_t *data = (thread_data_t *)arg;
  struct crypt_data crypt_data;
  crypt_data.initialized = 0;
  char password[MAX_PASSWORD_LEN];

  while (!found) {
    pthread_mutex_lock(&dict_mutex);
    if (fgets(password, MAX_PASSWORD_LEN, data->dictionary) == NULL) {
      pthread_mutex_unlock(&dict_mutex);
      break;
    }
    pthread_mutex_unlock(&dict_mutex);

    // Remove newline character
    password[strcspn(password, "\n")] = '\0';

    char salt_formatted[256];
    snprintf(salt_formatted, sizeof(salt_formatted), "$6$%s$", data->salt);

    const char *computed_hash = crypt_r(password, data->hash, &crypt_data);

    if (strcmp(computed_hash, data->hash) == 0) {
      printf("Password found: %s\n", password);
      found = 1;
      return NULL;
    }
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <hash> <dictionary_file> <num_threads>\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  char *hash = argv[1];
  const char *dictionary_path = argv[2];
  int num_threads_requested = atoi(argv[3]);

  // Extract the salt from the hash
  char *salt_start = strchr(hash, '$');
  if (!salt_start) {
    fprintf(stderr, "Invalid hash format: missing $id$\n");
    return EXIT_FAILURE;
  }
  salt_start += 1; // Skip past the first '$'
  salt_start = strchr(salt_start, '$');
  if (!salt_start) {
    fprintf(stderr, "Invalid hash format: missing salt delimiter\n");
    return EXIT_FAILURE;
  }
  salt_start += 1; // Skip past the second '$'
  const char *salt_end = strchr(salt_start, '$');
  const size_t salt_len = salt_end ? (salt_end - salt_start) : strlen(salt_start);
  char salt[salt_len + 1];
  strncpy(salt, salt_start, salt_len);
  salt[salt_len] = '\0';

  const int num_processors = sysconf(_SC_NPROCESSORS_ONLN);
  if (num_threads_requested > num_processors) {
    num_threads_requested = num_processors;
  }

  FILE *dictionary = fopen(dictionary_path, "r");
  if (!dictionary) {
    perror("Error opening dictionary file");
    return EXIT_FAILURE;
  }

  pthread_t threads[num_threads_requested];
  thread_data_t thread_data[num_threads_requested];

  for (int i = 0; i < num_threads_requested; i++) {
    thread_data[i].hash = hash;
    thread_data[i].salt = salt;
    thread_data[i].dictionary = dictionary;
    thread_data[i].thread_num = i;
    thread_data[i].total_threads = num_threads_requested;
    if (pthread_create(&threads[i], NULL, crack_password, &thread_data[i]) !=
        0) {
      perror("Error creating thread");
      fclose(dictionary);
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < num_threads_requested; i++) {
    pthread_join(threads[i], NULL);
  }

  fclose(dictionary);

  if (!found) {
    printf("Password not found in dictionary.\n");
  }

  return EXIT_SUCCESS;
}
