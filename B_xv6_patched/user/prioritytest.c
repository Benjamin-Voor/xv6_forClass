#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define NUM_PROCS 5

int
main(void)
{
    int pids[NUMS_PROCS];
    int priorities[NUM_PROCS] = {10, 30, 50, 100, 200}

    for(int i = 0; i < NUM_PROCS; i++) {
        int pid = fork();
        if(pid == 0) {
            setpriority(priorities[i]);
            volatile int j;
            for(;;) for(j = 0; j < 1000000; j++); // Burn CPU
        }
        else {
            pids[i] = pid;
        }
    }
    sleep(2000);

    struct pstat ps;
    if(getinfo(&ps) < 0) {
        printf(1, "getpinfo failed\n");
        exit();
    }

    printf(1, "PID\tPriority\tTicks\n");
    for(int i = 0; i < NPROC; i++) {
        if(ps.inuse[i]) {
            for(int j = 0; j < NUM_PROCS; j++) {
                if(ps.pid[i] == pids[j]) {
                    printf(1, "%d\t%d\t\t%d\n", ps.pid[i], priorities[j], ps.ticks[i]);
                }
            }
        }
    }

    for(int i = 0; i < NUM_PROCS; i++)
        kill(pids[i]);

    exit();
}