#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "ptable.h"
#include "pstat.h"

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
  return counterB;
}

int
sys_PartC(void)
{
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
sys_setpriority(void)
{
  int priority;
  
  if(argint(0, &priority) < 0)
    return -1;

  if(priority < 0 || priority > 200)
    return -1;

  struct proc *p = myproc();
  int old_priority;

  acquire(&ptable.lock);
  old_priority = p->priority;
  p->priority = priority;
  release(&ptable.lock);

  // If new priority is lower (larger value), yield
  if(priority > old_priority){
    yield();
  }

  return old_priority;
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

int
sys_ps(void)
{
  return ps(); // implemented in kernel/proc.c
}


int sys_getpinfo(void) {
  struct pstat *pInfo;
  if(argptr(0, (void *)&pInfo, sizeof(*pInfo)) < 0) {
    return -1;
  }
  if(pInfo == NULL) {
    return -1;
  }
  getpinfo(pInfo);
  return 0;
}