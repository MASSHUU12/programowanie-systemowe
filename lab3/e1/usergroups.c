#include "usergroups.h"
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_user_groups(const uid_t uid) {
  struct passwd *pw = getpwuid(uid);
  if (pw == NULL) {
    return NULL;
  }

  int ngroups = 0;
  getgrouplist(pw->pw_name, pw->pw_gid, NULL, &ngroups);

  gid_t *groups = malloc(ngroups * sizeof(gid_t));
  if (groups == NULL) {
    return NULL;
  }

  if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1) {
    free(groups);
    return NULL;
  }

  size_t buffer_size = 256;
  char *buffer = malloc(buffer_size);
  if (buffer == NULL) {
    free(groups);
    return NULL;
  }

  strcpy(buffer, "[");

  struct group *gr;
  for (int i = 0; i < ngroups; i++) {
    gr = getgrgid(groups[i]);
    if (gr == NULL) {
      continue;
    }

    size_t len = strlen(buffer);
    size_t group_name_len = strlen(gr->gr_name);

    if (len + group_name_len + 4 >= buffer_size) {
      buffer_size *= 2;
      char *new_buffer = realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        free(buffer);
        free(groups);
        return NULL;
      }
      buffer = new_buffer;
    }

    if (i > 0) {
      strcat(buffer, ", ");
    }
    strcat(buffer, gr->gr_name);
  }

  strcat(buffer, "]");
  free(groups);

  return buffer;
}
