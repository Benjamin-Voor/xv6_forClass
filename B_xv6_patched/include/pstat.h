# ifndef _PSTAT_H_
# define _PSTAT_H_
# include "param.h"

struct pstat {
    int inuse[NPROC]; // whether this slot is in use (1 or 0)
    int pid[NPROC]; // PID of each process
    int ticks[NPROC]; // number of ticks process was scheduled
    int size[NPROC]; // number of bytes of the process
    int priority[NPROC]; // priority of the process
};

# endif // _PSTAT_H_