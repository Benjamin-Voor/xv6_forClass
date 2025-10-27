#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "kernel/pstat.h"
#include "spinlock.h"


extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int counterA = 0;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
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
sys_getpid(void)
{
  counterA++;
  return proc->pid;
}

int
sys_PartA(void)
{
  return counterA;
}

int
sys_PartB(void)
{
  extern int counterB; // declared in defs.h
  return counterB;
}

int
sys_PartC(void)
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

// return how many clock tick interrupts have occurred since boot.
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
sys_ps(void)
{
  return ps(); // implemented in kernel/proc.c
}

// ===================== Part A: Lottery syscalls =====================

int
sys_settickets(void)
{
  int n;
  if (argint(0, &n) < 0) return -1;
  if (n < 1) return -1;

  acquire(&ptable.lock);
  // Use the global 'proc' in this xv6 tree.
  proc->tickets = n;
  release(&ptable.lock);
  return 0;
}

int
sys_getpinfo(void)
{
  struct pstat *ps;
  int i;

  if (argptr(0, (void*)&ps, sizeof(*ps)) < 0)
    return -1;

  acquire(&ptable.lock);
  for (i = 0; i < NPROC; i++) {
    struct proc *p = &ptable.proc[i];
    if (p->state == UNUSED) {
      ps->inuse[i]   = 0;
      ps->tickets[i] = 0;
      ps->pid[i]     = 0;
      ps->ticks[i]   = 0;
    } else {
      ps->inuse[i]   = 1;
      ps->tickets[i] = p->tickets;
      ps->pid[i]     = p->pid;
      ps->ticks[i]   = p->ticks;
    }
  }
  release(&ptable.lock);
  return 0;
}
