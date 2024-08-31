#ifndef _THREADS_H
#define _THREADS_H

#include "atomics.h"
#include "types.h"
#include "user.h"

/** Threads */

int thread_create(void *(*start_routine)(void *), void *arg);
int thread_wait(int pid);

/** Spinlock */

struct spinlock {
  // You may add fields if needed
  atomic_uint locked;
};

int spinlock_init(struct spinlock *);
int spinlock_acquire(struct spinlock *);
int spinlock_release(struct spinlock *);

/** Mutex */

struct mutex {
  // You may add fields if needed
  int holder_pid;
  atomic_uint locked;
  struct spinlock lock;
};

int mutex_init(struct mutex *);
int mutex_acquire(struct mutex *);
int mutex_release(struct mutex *);

/** Cond var */

struct condvar {
  // You may add fields if needed
  atomic_uint signal;
};

int cond_init(struct condvar *);
int cond_wait(struct condvar *, struct mutex *);
int cond_signal(struct condvar *);

#endif  // _THREADS_H
