/* Nvm, it wouldn't work. No rule to make spinlock.
// part A only // Mini-Project 2
// Previously, this struct was only declared in proc.c, but sys_settickets() needs it, so here's a header file to include both!
#ifndef _PTABLE_H_
#define _PTABLE_H_

#include "spinlock.h" // This has an ifndef line at the top, so it shouldn't be a problem to include this twice
#include "param.h" // NPROC
#include "proc.h" // struct proc

struct {
  struct spinlock lock; // #include "spinlock.h"
  struct proc proc[NPROC]; // #include "param.h" #include "proc.h"
} ptable; // Mini-project 2

#endif
*/