#include "types.h"
#include "user.h"

void busywork(int n) {
    int i, j;
    for(i = 0; i < n; i++)
        for(j = 0; j < 1000000; j++)
            ; // busy loop
}

int main(void) {
    int pid1 = fork();
    if(pid1 == 0) {
        setpriority(getpid(), 10);
        for(int i = 0; i < 5; i++) {
            printf(1, "Child 1 (priority=10) running\n");
            busywork(5);
        }
        exit();
    }

    int pid2 = fork();
    if(pid2 == 0) {
        setpriority(getpid(), 50);
        for(int i = 0; i < 5; i++) {
            printf(1, "Child 2 (priority=20) running\n");
            busywork(5);
        }
        exit();
    }

    // Parent waits for children
    wait();
    wait();
    exit();
}