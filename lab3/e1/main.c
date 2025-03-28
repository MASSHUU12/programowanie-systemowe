#include "usergroups.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <utmpx.h>

int main(int argc, char *argv[]) {
  int opt, show_host = 0, show_groups = 0;

  while ((opt = getopt(argc, argv, "hg")) != -1) {
    switch (opt) {
    case 'h':
      show_host = 1;
      break;
    case 'g':
      show_groups = 1;
      break;
    default:
      fprintf(stderr, "Usage: %s [-h] [-g]\n", argv[0]);
      return EXIT_FAILURE;
    }
  }

  struct utmpx *u;
  while ((u = getutxent()) != NULL) {
    if (u->ut_type != USER_PROCESS) {
      u = getutxent();
      continue;
    }

    struct passwd *p = getpwnam(u->ut_user);
    if (p == NULL) {
      u = getutxent();
      continue;
    }

    printf("%s", p->pw_name);

    if (show_host && strlen(u->ut_host) > 0) {
      printf("(%s)", u->ut_host);
    }

    if (show_groups) {
      char *groups = get_user_groups(p->pw_uid);
      if (groups != NULL) {
        printf(" %s", groups);
        free(groups);
      }
    }

    printf("\n");
  }

  endutxent();
  return 0;
}
