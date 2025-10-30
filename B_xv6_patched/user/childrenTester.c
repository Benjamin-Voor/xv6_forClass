#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[]) {
    int i;
    int rc;

    printf(1, "Parent PID: %d\n", getpid());
    for(i = 1; i <= 3; i++) {
        rc = fork();
        if(!rc) {
            // Child process
            printf(1, "Child %d PID: %d\n", i, getpid());
            for(;;){}
        }
    }
    exit();
}