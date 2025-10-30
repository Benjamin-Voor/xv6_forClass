#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "pstat.h" // baseline-1.pdf // I assume
#include "spinlock.h"

int syscall_counter = 0; // Mini-project 1

/* Mini-project 2 */
// #include <stdlib.h> // "rand()" for lottery scheduler // failing because of kernel/makefile.mk:68
static unsigned long rand_next = 1;
int
random(void)
{
  rand_next *= 1103515245 + 12345;
  return (unsigned int)(rand_next / 65536) % 32768;
}

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  { // baseline-1.pdf has no braces here, but it makes sense, yaknow?
    if(p->state == UNUSED)
      goto found;
  }
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  // Mini-Project 1, Part c
  p->syscallCount = 0;   // Initialize your part C counter  for initialize syscall counter

  // Mini-project 2, Part A
  p->numTicks = 0; // baseline-1.pdf
  p->tickets = 1;   // each proc starts with one ticket
  p->ticks = 0;     // no CPU time yet

  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    // Need the lock to safely touch p->state.
    acquire(&ptable.lock);
    p->state = UNUSED;
    release(&ptable.lock);
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  // Context to start at forkret
  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();               // <-- get a proc FIRST
  if(p == 0)
    panic("userinit: allocproc failed");
  
  initproc = p;

  // Set up page table and first user page.
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;   // beginning of initcode.S

  // (Optional – allocproc already did this, but harmless to keep)
  p->tickets = 1;
  p->ticks   = 0;

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  acquire(&ptable.lock);
  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // // In fork(), after acquiring lock and before copying state
  // np->tickets = curproc->tickets;

  np->tickets = proc->tickets;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    acquire(&ptable.lock);
    np->state = UNUSED;
    release(&ptable.lock);
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  // Inherit scheduling params
  np->tickets = proc->tickets;  // child inherits parent's tickets
  np->ticks   = 0;              // fresh CPU time

  pid = np->pid;
  safestrcpy(np->name, proc->name, sizeof(proc->name));

  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process round-robin scheduler (you can later swap in lottery).
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  // struct cpu *c; // Don't need it. Usages must be commented out or removed.
  // *c = mycpu(); 
  cpu->proc = 0; // ¿Maybe cpu->proc = 0?

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);


    // Calculate total tickets from RUNNABLE processes
    int total_tickets = 0; // part A only // Mini-project 2
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state == RUNNABLE)
        total_tickets += p->tickets; // part A only // Mini-project 2
    }

    if(total_tickets > 0){ // Mini-Project 2, Part A, Step 3. This whole if-statement could have been inverted!
      // Hold lottery
      int winner = random() % total_tickets; // "random()" implemented in kernproc.c

      // Find winning process
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state == RUNNABLE){
          winner -= p->tickets;
          if(winner < 0){
            // This process wins!
            cpu->proc = p; // ¿Maybe cpu->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            p->ticks++;   // Increment schedule count // part A only
            p->numTicks += 1; // baseline-1.pdf
            
            swtch(&cpu->scheduler, p->context); // "&(scheduler)" error: passing argument 1 of ‘swtch’ from incompatible pointer type
            switchkvm();

            cpu->proc = 0;
            break;
          }
        }
      }
    }

    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

struct proc*
myproc(void)
{
  return proc;   // using the global directly is fine in this codebase
}


int 
ps(void) // baseline-1.pdf
{
  struct proc *p;
  char *state;

  cprintf("PID\tState\t\tMemory Size\t\tProcess Name\t\tTickets\n");

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; ++p) {
    if(p->state == UNUSED) {
      continue;
    } 
    if(p->state == SLEEPING) {
      state = "SLEEPING";
    } 
    else if (p->state == RUNNING) {
      state = "RUNNING";
    }
    else if (p->state == ZOMBIE) {
      state = "ZOMBIE";
    }
    else if (p->state == EMBRYO)
    {
      state = "EMBRYO";
    }
    else {
      state = "OTHER";
    }
    cprintf("PID: %d\tState: %s\tMemory Size: %d\tProcess Name: %s\tTickets: %d\n",
            p->pid, state, p->sz, p->name, p->tickets);
  }
  release(&ptable.lock);

  return 0;
}

int
getpinfo(struct pstat* pInfo) // baseline-1.pdf
{
  struct proc *p; 
  int i=0;
  acquire(&ptable.lock);
  for(p=ptable.proc; p < &ptable.proc[NPROC]; p++, i++) {
    if (p->state==ZOMBIE || p->state == EMBRYO) {
      continue;
    }
    else if(p->state == UNUSED) {
      pInfo->inuse[i] = 0; // #include "pstat.h"
    } // PROCESS VARIABLES
    else {
      pInfo->inuse[i] = 1;
    }
    // cprintf("%d, ", p->state); // debugging
    pInfo->pid[i] = p->pid;
    // cprintf("%d, ", pInfo->pid[i]); // debugging
    pInfo->ticks[i] = p->ticks;
    // cprintf("%d, ", pInfo->ticks[i]); // debugging
    pInfo->size[i] = p->sz; // error: 'struct proc' has no member named 'size'; did you mean 'sz'?
    pInfo->tickets[i] = p->tickets;
    // cprintf("%d ", p->tickets); // debugging
    // i++; // This is done within the for loop
    // cprintf("%d, ", i);
  }
  release(&ptable.lock);
  return 0;
}

// Prototype is in sysproc.c
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

  acquire(&ptable.lock); // This line gives me the error that identifier "ptable" is undefined. It's defined in proc.c, not in sysproc.c
  myproc()->tickets = n;
  // cprintf("%d, ", cpu->proc->tickets); // debugging
  release(&ptable.lock);

  return 0;
}

/* No regerts
// Prototype is in sysproc.c
int
sys_getpinfo(void) // Mini-Project 2 // part A only, Lottery Scheduler
{
  struct pstat *ps;

  if (argptr(0, (void *)&ps, sizeof(*ps)) < 0) {
    return -1;
  }
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

  release(&ptable.lock);
  return 0;
}
*/