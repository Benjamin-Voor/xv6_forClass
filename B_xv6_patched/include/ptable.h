#ifndef _PTABLE_H_
#define _PTABLE_H_

#include "spinlock.h"
#include "param.h" // for NPROC
#include "proc.h" // struct proc

struct ptable_struct {
    struct spinlock lock; // lock for modifying process table
    struct proc proc[NPROC]; // process table
};

extern struct ptable_struct ptable; // declare global variable ptable (defined in proc.c)

#endif
