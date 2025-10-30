// TODO: graph

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
#include "stat.h" // I saw it in baseline for a different tester
// #include "proc.c" // myproc()->pid // Error: No rule to make target proc.c
// #include "proc.h" //cpu->proc->pid // Error: No rule to make target proc.h


int
main(void)
{
    struct pstat st;
    int pid;
    int tickets[] = {10, 20, 30, 40, 50}; // Rubric entry #2
    int n = 5;

    printf(1, "Starting tickettest...\n");
    // printf(1, "Debugging: int tickets[] = {"); // debugging
    for (int i = 0; i < n; i++) {
        pid = fork(); // Rubric entry #1
        if (pid == 0) {
            settickets(tickets[i]);
            // Busy work loop // Rubric entry #3
            for (volatile int j = 0; j < 10000000; j++);
            exit(); // TODO: continue making forks until you get to 5
        }
    }
    for (int i = 0; i < n; i++)
        wait();
    // sleep(100); // Nvm, AI is stupid!
    // printf(1, "}\n"); // debugging
    // printf(1, "p->tickets: {"); // debugging
    if (getpinfo(&st) < 0) { // Rubric entry #4
        printf(1, "getpinfo failed\n");
        exit();
    }
    // printf(1, "}\n"); // debugging



    printf(1, "PID, Tickets, Ticks\n");
    for (int i = 3; i < NPROC; i++) {
        // if (! st.inuse[i]) { continue; }
        if (! (st.ticks[i] || st.tickets[i])) { continue; }
        printf(1, "%d, %d, %d\n", i, st.tickets[i], st.ticks[i]);
    }

    // exit(0); // No, it's a void func
    exit();
}
