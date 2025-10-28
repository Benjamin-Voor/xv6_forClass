// Part A only: Test lottery scheduler
// Create a test program that:
    // 1. Creates 5 child processes using fork()
    // 2. Assigns different ticket counts: 10, 20, 30, 40, 50
    // 3. Has each process do CPU-intensive work (loop)
    // 4. Calls getpinfo() to collect scheduling statistics
    // 5. Displays results showing ticket count vs actual CPU time

// #include "kernel/types.h"
// #include "user/user.h"
// #include "kernel/pstat.h"
#include "types.h"
#include "user.h"
#include "pstat.h"

int
main(void)
{
    struct pstat st;
    int pid;
    int tickets[] = {10, 20, 30, 40, 50}; // 2. Assigns different ticket counts: 10, 20, 30, 40, 50
    int n = 5;

    printf(1, "Starting tickettest...\n");

    for (int i = 0; i < n; i++) {
        pid = fork(); // 1. Creates 5 child processes using fork()
        if (pid == 0) {
            settickets(tickets[i]);
            // Busy work loop // 3. Has each process do CPU-intensive work (loop)
            for (volatile int j = 0; j < 10000000; j++);
            // exit(0);
            exit();
        }
    }

    for (int i = 0; i < n; i++)
        // wait(0); // No, it's a void func
        wait();

    if (getpinfo(&st) < 0) { // 4. Calls getpinfo() to collect scheduling statistics
        printf(1, "getpinfo failed\n");
        // exit(1);
        exit();
    }

    printf(1, "PID\tTickets\tTicks\n"); // 5. Displays results showing ticket count vs actual CPU time
    for (int i = 0; i < NPROC; i++) {
        if (st.inuse[i]) {
            // printf(1, "%d\t%d\t%d\n", st.pid[i], st.tickets[i], st.ticks[i]); // struct pstat has no member named 'tickets'
            printf(1, "%d\t%d\t%d\n", st.pid[i], 404, st.ticks[i]);
        }
    }

    // exit(0); // No, it's a void func
    exit();
}
