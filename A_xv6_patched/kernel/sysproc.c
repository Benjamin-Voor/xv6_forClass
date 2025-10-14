#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

int counterA = 0;

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  counterA++;
  return proc->pid;
}

int sys_PartA(void)
{
  return counterA;
}

int sys_PartB(void)
{
  return counterB;
}

int sys_PartC(void)
{
  return counterC;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (proc->killed)
    {
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
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int syscall_counter; // part c

int sys_thirdpart(void)
{
  return syscall_counter;
}
int sys_ps(void) // Mini-Project 2, Option 1 (no longer pursued), Part A
{
  return ps(); // implemented in kernel/proc.c
}

int sys_settickets(void) // Mini-Project 2, Part A
{
  int n;

  if (argint(0, &n) < 0)
  {
    return -1;
  }

  if (n < 1)
  {
    return -1;
  }

  acquire(&ptable.lock); // This line gives me the error that identifier "ptable" is undefined
  myproc()->tickets = n;
  release(&ptable.lock);

  return 0;
}

int sys_getpinfo(void)
{
  struct pstat *ps;
  if (argptr(0, (void *)&ps, sizeof(*ps)) < 0)
    return -1;
  struct proc *p;
  int i = 0;
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++, i++)
  {
    if (p->state == UNUSED)
    {
      ps->inuse[i] = 0;
    }
    else
    {
      ps->inuse[i] = 1;
      ps->tickets[i] = p->tickets;
      ps->pid[i] = p->pid;
      ps->ticks[i] = p->ticks;
    }
  }
}
