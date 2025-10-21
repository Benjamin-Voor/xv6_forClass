#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void) {
    int pid;

    // Create multiple processes with different priorities
    for(int i = 0; i < 5; i++) {
        pid = fork();

        if(pid < 0){
            printf("Fork failed\n");
            exit();
        }

        if(pid == 0) {
            int prio = (i+1) * 20;  // priorities: 20, 40, 60, 80, 100
            int old = setpriority(prio);
            printf("Child %d: old priority = %d, new priority = %d\n", getpid(), old, prio);

            // Consume CPU time
            volatile int j;
            for(j = 0; j < 100000000; j++);
                
            printf("Child %d finished\n", getpid());
            exit();
        }
    } 

    // Parent waits for all children
    for(int i = 0; i < 5; i++)
        wait();

    printf("Priority test completed.\n");
    exit();
}