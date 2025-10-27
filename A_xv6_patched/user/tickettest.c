#include "kernel/types.h"
#include "user/user.h"
#include "kernel/pstat.h"




int
main(void)
{
    struct pstat st;
    int pid;
    int tickets[] = {10, 20, 30, 40, 50};
    int n = 5;

    printf("Starting tickettest...\n");

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            settickets(tickets[i]);
            // Busy work loop
            for (volatile int j = 0; j < 10000000; j++);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++)
        wait(0);

    if (getpinfo(&st) < 0) {
        printf("getpinfo failed\n");
        exit(1);
    }

    printf("PID\tTickets\tTicks\n");
    for (int i = 0; i < NPROC; i++) {
        if (st.inuse[i]) {
            printf("%d\t%d\t%d\n", st.pid[i], st.tickets[i], st.ticks[i]);
        }
    }

    exit(0);
}
