#include <dlfcn.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <utmpx.h>

typedef char *(*get_user_groups_func)(uid_t);

int main(int argc, char *argv[]) {
  int opt, show_host = 0, show_groups = 0;
  void *lib_handle = NULL;
  get_user_groups_func get_groups_ptr = NULL;

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

  if (show_groups) {
    lib_handle = dlopen("./libusergroups.so", RTLD_LAZY);
    if (!lib_handle) {
      fprintf(stderr, "Error: Could not load library: %s\n", dlerror());
      show_groups = 0;
    } else {
      get_groups_ptr =
          (get_user_groups_func)dlsym(lib_handle, "get_user_groups");
      if (!get_groups_ptr) {
        fprintf(stderr,
                "Error: Could not find function 'get_user_groups': %s\n",
                dlerror());
        dlclose(lib_handle);
        show_groups = 0;
      }
    }
  }

  struct utmpx *u = getutxent();
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

    if (show_groups && get_groups_ptr) {
      char *groups = get_groups_ptr(p->pw_uid);
      if (groups != NULL) {
        printf(" %s", groups);
        free(groups);
      }
    }

    printf("\n");
  }

  endutxent();

  if (lib_handle) {
    dlclose(lib_handle);
  }

  return 0;
}
