#include <stdlib.h>
#include <stdio.h>
#include <utmpx.h>
#include <sys/types.h>
#include <pwd.h>

int main(void) {
    struct utmpx* u =  getutxent();

    while(u != NULL) {
        if (u->ut_type != USER_PROCESS) {
           u = getutxent();

           continue;
        }

        struct passwd* p = getpwnam(u->ut_user);
        printf("%d %s %s %s\n", p->pw_uid, p->pw_name, u->ut_line, u->ut_host);

        u = getutxent();
    }

    return 0;
}
