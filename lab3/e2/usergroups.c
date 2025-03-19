#include "usergroups.h"
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

char* get_user_groups(uid_t uid) {
    struct passwd* pw = getpwuid(uid);
    if (pw == NULL) {
        return NULL;
    }

    // Initial allocation for group IDs
    int ngroups = 10;
    gid_t* groups = malloc(ngroups * sizeof(gid_t));
    if (groups == NULL) {
        return NULL;
    }

    // Get the groups
    if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1) {
        // Reallocate with the correct size
        free(groups);
        groups = malloc(ngroups * sizeof(gid_t));
        if (groups == NULL) {
            return NULL;
        }
        getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);
    }

    // Allocate a buffer for the group names
    size_t buffer_size = 256;
    char* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        free(groups);
        return NULL;
    }

    strcpy(buffer, "[");

    // Get the group names and append to buffer
    struct group* gr;
    for (int i = 0; i < ngroups; i++) {
        gr = getgrgid(groups[i]);
        if (gr != NULL) {
            size_t len = strlen(buffer);
            size_t group_name_len = strlen(gr->gr_name);

            // Resize buffer if needed
            if (len + group_name_len + 4 >= buffer_size) {  // +4 for ", " and potential "]"
                buffer_size *= 2;
                char* new_buffer = realloc(buffer, buffer_size);
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
    }

    strcat(buffer, "]");
    free(groups);

    return buffer;
}
