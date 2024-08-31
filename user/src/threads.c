#include "threads.h"
#include "atomics.h"
#include "user.h"
#include "free_stack_and_exit.h"
#include "memlayout.h"
#include "mmu.h"

// Thread Create
// Arguments:
//   - start_routine -- A function pointer to the routine that the child thread
//   will run
//   - arg -- the argument passed to start_routine
// Return Value:
//   - -1 on failure, pid of the created thread on success
// Description:
// Creates a new child thread.  That thread will immediately begin running
// start_routine, as though invoked with start_routine(arg).
int thread_create(void* (*start_routine)(void*), void* arg) {
  // Check if the function is in user space (this is a userspace library after
  // all)
  if ((void*)start_routine >= ((void*)KERNBASE)) {
    return -1;
  }
  // Check if arguments are in user space
  if (arg >= ((void*)KERNBASE)) {
    return -1;
  }

  // Each thread has a private:
  // - stack (kernel and user)
  // - set of registers
  // clone() will take care of the registers (via the tf) and the kstack (via
  // allocproc). We just need to malloc a user stack for the new thread.
  void* stack = malloc(PGSIZE);
  // Check for malloc failure
  if (stack == (void*)0) {
    return -1;
  }

  // There are 3 possiblities here to consider.
  // 1. new_pid == -1: clone failed. Free the stack and return -1
  // 2. new_pid != 0: parent thread. Return the pid and nothing else
  // 3. new_pid == 0: child thread. Call start_routine and free the stack once
  // done
  int new_pid = clone(stack, PGSIZE);

  // Case 1: Clone Failed
  if (new_pid == -1) {
    free(stack);
    return -1;
  }

  // Case 2: Parent Thread
  if (new_pid != 0) {
    return new_pid;
  }

  // Case 3: Child thread
  if (new_pid == 0) {
    start_routine(arg);
    free_stack_and_exit(stack);
    return new_pid;
  }

  return -1;
}

// Thread Wait
// Arguments:
//   - pid -- pid of the thread to wait on
// Return Value:
//   - -1 on failure, pid of the joined thread on success
// Description:
// Waits for a child thread of process id = pid to finish.
int thread_wait(int pid) { return waitpid(pid); }

int spinlock_init(struct spinlock* s) {
  if (s != 0) {
    atomic_init(&s, 0);
    return 0;
  }
  return -1;
}

int spinlock_acquire(struct spinlock* s) {
  if (s != 0) {
    while (atomic_exchange_explicit(&s->locked, 1, memory_order_acq_rel) != 0)
      ;
    return 0;
  }
  return -1;
}

int spinlock_release(struct spinlock* s) {
  if (s != 0 && s->locked) {
    atomic_store_explicit(&s->locked, 0, memory_order_release);
  }
  return -1;
}

// Mutex Init
// Arguments:
//   - m -- a pointer to the mutex to initialize
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Initializes a mutex.
int mutex_init(struct mutex* m) {
  // Fails if mutex doesn't exists
  if (!m) {
    // Create the mutex using atomic inits + spinlock init
    atomic_init(&m->locked, 0);
    spinlock_init(&m->lock);
    // Set PID to an invalid value by default
    atomic_init(&m->holder_pid, -1);
    return 0;
  }
  return -1;
}

// Mutex Acquire
// Arguments:
//   - m -- a pointer to the mutex to acquire
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Acquires a mutex. If the mutex is already locked, the calling thread will
// sleep until the mutex is available.
int mutex_acquire(struct mutex* m) {
  // Fails if mutex doesn't exists
  if (!m) {
    return -1;
  }
  // Check if mutex is already locked by the calling thread
  if (m->holder_pid == getpid()) {
    return -1;
  }
  spinlock_acquire(&m->lock);
  while (m->locked) {
    // Do good practice and setpark before parking to show intention.
    // If this mutex is locked then we go to sleep.
    setpark(m);
    spinlock_release(&m->lock);
    park(m);
    spinlock_acquire(&m->lock);
  }
  // Acquire the mutex, and return to user so they can safely execute CCP
  atomic_store_explicit(&m->locked, 1, memory_order_release);
  atomic_store_explicit(&m->holder_pid, getpid(), memory_order_release);
  spinlock_release(&m->lock);
  return 0;
}

// Mutex Release
// Arguments:
//   - m -- a pointer to the mutex to release
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Releases a mutex. If there are any threads waiting on the mutex, one of them
// will be woken up
int mutex_release(struct mutex* m) {
  // Fail if mutex doesn't exist, or we are trying to release someone elses'
  // lock
  if (m && m->holder_pid == getpid()) {
    // Acquire the spinlock, and update values
    spinlock_acquire(&m->lock);
    atomic_store_explicit(&m->holder_pid, -1, memory_order_release);
    atomic_store_explicit(&m->locked, 0, memory_order_release);
    unpark(m);
    spinlock_release(&m->lock);
    return 0;
  }
  return -1;
}

// cond_init
// Arguments:
//   - cond -- a pointer to the condition variable to initialize
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Initializes a condition variable.
int cond_init(struct condvar* cond) {
  if (cond) {
    atomic_init(&cond->signal, 0);
    return 0;
  }
  return -1;
}

// cond_wait
// Arguments:
//   - cond -- a pointer to the condition variable to wait on
//   - m -- a pointer to the mutex to acquire
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Atomically blocks the current thread waiting on the condition variable cond,
// and releases the mutex m. The waiting thread unblocks only after another
// thread calls cond_signal. After being woken up the current thread reacquires
// the mutex m.
int cond_wait(struct condvar* cond, struct mutex* m) {
  if (m && cond) {
    while (!cond->signal) {
      setpark(cond);
      mutex_release(m);
      park(cond);
      mutex_acquire(m);
    }
    atomic_store_explicit(&cond->signal, 0, memory_order_release);
    return 0;
  }
  return -1;
}

// cond_signal
// Arguments:
//   - cond -- a pointer to the condition variable to signal
// Return value:
//   - -1 on failure, 0 on success
// Description:
// Unblocks one thread waiting for the condition variable cond.
int cond_signal(struct condvar* cond) {
  if (cond) {
    atomic_store_explicit(&cond->signal, 1, memory_order_release);
    unpark(cond);
    return 0;
  }
  return -1;
}
