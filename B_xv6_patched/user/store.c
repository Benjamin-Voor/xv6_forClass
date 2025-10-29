#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

// Outputs results of getpinfo in a reable way
// Options:
// a: Abbreviate. Do not print unused processes.
int main(int argc, char *argv[]) {
    int i;
    struct pstat proc_info;
    int abbreviate = 0;
    for(i = 0; i < argc; i++) {
        if(*argv[i] == 'a')
            abbreviate = 1;
    }
    getpinfo(&proc_info);
    for(i = 0; i < NPROC; i++) {
        if(abbreviate && !proc_info.inuse[i])
            continue;
        printf(1,
            "Use: %d   Size: %d   PID: %d   Ticks: %d\n",
            proc_info.inuse[i],
            proc_info.size[i],
            proc_info.pid[i],
            proc_info.ticks[i]);
    }
    exit();
}