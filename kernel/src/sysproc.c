#include "asm/x86.h"
#include "types.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void) { return fork(); }

int sys_exit(void) {
  exit();
  return 0;  // not reached
}

int sys_wait(void) { return wait(); }

int sys_kill(void) {
  int pid;

  if (argint(0, &pid) < 0) return -1;
  return kill(pid);
}

int sys_getpid(void) { return myproc()->pid; }

int sys_sbrk(void) {
  int addr;
  int n;

  if (argint(0, &n) < 0) return -1;
  acquire(&myproc()->owner->grouplock);
  addr = myproc()->owner->sz;
  release(&myproc()->owner->grouplock);
  if (growproc(n) < 0) return -1;
  return addr;
}

int sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0) return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// wrapper for setscheduler in proc.c
// modify a process's priority or scheduling algorithm
// return 0 on success, -1 on failure
int sys_setscheduler(void) {
  int pid, policy, priority;
  argint(0, &pid);
  argint(1, &policy);
  argint(2, &priority);
  int result = setscheduler(pid, policy, priority);
  return result;
}

int sys_clone(void) {
  void *stack;
  int stack_size;
  argint(1, &stack_size);
  if (argptr(0, (char **)&stack, stack_size) < 0) {
    cprintf("!!! Stack ptr was not fetched properly !!!\n");
  }
  return clone(stack, stack_size);
}

int sys_waitpid(void) {
  int pid;
  argint(0, &pid);
  return waitpid(pid);
}

int sys_waitinfo(void) {
  struct schedinfo *si;
  argptr(0, (char **)&si, (int)sizeof(struct schedinfo *));
  return waitinfo(si);
}

int sys_park(void) {
  int chan;
  argint(0, &chan);
  return park((void *)chan);
}

int sys_unpark(void) {
  int chan;
  argint(0, &chan);
  return unpark((void *)chan);
}

int sys_setpark(void) {
  int chan;
  argint(0, &chan);
  return setpark((void *)chan);
}
