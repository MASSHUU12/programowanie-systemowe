#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <password> <salt>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *password = argv[1];
  const char *salt = argv[2];
  char salt_formatted[256];

  snprintf(salt_formatted, sizeof(salt_formatted), "$6$%s$", salt);

  struct crypt_data crypt_data;
  crypt_data.initialized = 0;

  const char *hashed_password = crypt_r(password, salt_formatted, &crypt_data);
  if (hashed_password == NULL) {
    perror("crypt_r");
    return EXIT_FAILURE;
  }

  printf("Formatted hash for /etc/shadow: %s\n", hashed_password);

  return EXIT_SUCCESS;
}
