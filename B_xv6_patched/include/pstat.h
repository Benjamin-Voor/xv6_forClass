# ifndef _PSTAT_H_
# define _PSTAT_H_
# include "param.h"

struct pstat {
    int inuse[NPROC]; // whether this slot is in use (1 or 0)
    int tickets[NPROC]; // number of tickets this process has
    int pid[NPROC]; // PID of each process
    int ticks[NPROC]; // number of times process was scheduled
};

# endif // _PSTAT_H_