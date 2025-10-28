#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"
#include "spinlock.h"



int counterA = 0; // Mini-Project 1

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0; // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void) // Mini-project 1
{
  counterA++;
  return proc->pid;
}

int
sys_PartA(void) // Mini-project 1
{
  return counterA;
}

int
sys_PartB(void) // Mini-project 1
{
  extern int counterB; // declared in defs.h
  return counterB;
}

int
sys_PartC(void) // Mini-project 1
{
  extern int counterC; // declared in defs.h
  return counterC;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int syscall_counter; // part c

int
sys_thirdpart(void)
{
  return syscall_counter;
}
int
sys_ps(void) // baseline-1.pdf
{
  return ps(); // implemented in kernel/proc.c
}

// Implementation is in proc.c
int sys_settickets(void); // Mini-Project 2, Part A

// Implementation is in proc.c
int
sys_getpinfo(void); // Mini-Project 2, Part A