#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define NCHILD 8

int main(void) {
    int i;
    int rc;
    struct pstat ps;

    int priorities[NCHILD] = {10, 20, 30, 40, 50, 50, 50, 50};

    printf(1, "Parent PID: %d\n", getpid());

    // Step 1–2
    for (i = 0; i < NCHILD; i++) {
        rc = fork();
        if (rc == 0) {
            // Child process
            setpriority(priorities[i]);
            printf(1, "Child %d PID: %d running with priority %d\n",
                   i+1, getpid(), priorities[i]);

            // Step 3
            volatile int x = 0;
            for (int j = 0; j < 50000000; j++)
                x += j % 10;

            // Keep alive so results are shown in store
            while(1) sleep(100);
            exit();
        }
    }

    sleep(50);

    // Step 4–5
    getpinfo(&ps);

    printf(1, "\nPID\tPriority\tTicks\n");

    for (i = 0; i < NPROC; i++) {
        if (ps.inuse[i]) {
            printf(1, "%d\t%d\t\t%d\n",
                   ps.pid[i],
                   ps.priority[i],
                   ps.ticks[i]);
        }
    }
    exit();
}
