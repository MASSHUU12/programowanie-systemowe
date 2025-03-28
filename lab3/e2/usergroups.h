#ifndef USERGROUPS_H_
#define USERGROUPS_H_

#include <sys/types.h>

char* get_user_groups(const uid_t uid);

#endif // USERGROUPS_H
