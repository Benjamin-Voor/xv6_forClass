#ifndef _PTABLE_H_
#define _PTABLE_H_

#include "spinlock.h"
#include "param.h" // for NPROC
#include "proc.h" // struct proc

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

#endif