#include "types.h"
#include "stat.h"
#include "user.h"
#include "threads.h"
#include "sched.h"

void test0(void);
void test1(void);
void test2(void);
void test3(void);
void *child_thread(void *arg);
void test4(void);
void test5(void);

int main(int argc, char *argv[]) {
  if (argc > 0) {
    printf(1, "argc: %d, argv[1]: %c\n", argc, argv[1]);
    switch (atoi(argv[1])) {
      case 0:
        test0();
        break;
      case 1:
        test1();
        break;
      case 2:
        test2();
        break;
      case 3:
        test3();
        break;
      case 4:
        test4();
        break;
      case 5:
        test5();
      default:
        break;
    }
  }
  exit();
}

void test0(void) {
  printf(1, "* * * * TEST 0 * * * *\n");
  int pid = thread_create(child_thread, 0);
  for (int i = 0; i < 30; i++) {
    printf(1, "P\n");
  }
  printf(1, "parent exiting\n");
  waitpid(pid);
  exit();
}

void *child_thread(void *arg) {
  printf(1, "Child thread\n");
  for (int i = 0; i < 30; i++) {
    printf(1, "C\n");
  }
  printf(1, "Child exiting\n");
  exit();
}

void *locked_child_thread(void *arg) {
  struct spinlock *s = (struct spinlock *)arg;
  spinlock_acquire(s);
  printf(1, "Child thread\n");
  for (int i = 0; i < 30; i++) {
    printf(1, "C\n");
  }
  printf(1, "Child exiting\n");
  spinlock_release(s);
  exit();
}

void test1(void) {
  printf(1, "* * * * TEST 1 * * * *\n");
  struct spinlock *s = (struct spinlock *)malloc(sizeof(struct spinlock));
  spinlock_init(s);
  thread_create(locked_child_thread, s);

  for (int i = 0; i < 5; i++) {
    printf(1, "z\n");
  }

  spinlock_acquire(s);
  printf(1, "Parent thread\n");
  for (int i = 0; i < 30; i++) {
    printf(1, "P\n");
  }
  printf(1, "Parent exiting\n");
  spinlock_release(s);

  wait();
  exit();
}

void *child_with_priority(void *arg) {
  sleep(1);
  setscheduler(getpid(), SCHED_FIFO, 10);

  mutex_acquire((struct mutex *)arg);
  for (int i = 0; i < 50; i++) {
    printf(1, "C\n");
  }
  mutex_release((struct mutex *)arg);

  exit();
}

void test2(void) {
  printf(1, "* * * * TEST 2 * * * *\n");
  struct mutex *m = (struct mutex *)malloc(sizeof(struct mutex));
  mutex_init(m);
  thread_create(child_with_priority, m);

  mutex_acquire(m);
  for (int i = 0; i < 50; i++) {
    printf(1, "P\n");
  }
  mutex_release(m);

  wait();
  exit();
}

int x = 0;
void test3(void) {
  int y = 1000;
  printf(1, "user says hi\n");
  char *stack = malloc(4096);
  printf(1, "malloc-ed space on stack!\n");

  int rc = clone(stack, 4096);
  if (rc == -1) {
    printf(1, "clone failed\n");
  } else if (rc == 0) {
    printf(1, "child running!\n");
    x++;
    y *= 2;
    printf(1, "in child: x = %d, y = %d\n", x, y);
    exit();
  } else {
    waitpid(rc);
    printf(1, "parent running!\n");
    printf(1, "in parent: x = %d, y = %d\n", x, y);
  }
  exit();
}

void test4(void) {
  printf(1, "* * * * TEST 2 * * * *\n");
  char *stack = malloc(4096);
  char *stack2 = malloc(4096);

  int rc = clone(stack, 4096);
  if (rc == 0) {
    // Child 1
    sleep(100);
    park((void *)3);
    printf(1, "child 1 awake\n");
    unpark((void *)2);
    exit();
  } else if (rc > 0) {
    int rc2 = clone(stack2, 4096);
    if (rc2 == 0) {
      // Child 2
      sleep(200);
      unpark((void *)3);
      park((void *)1);
      printf(1, "all donee :)\n");
      exit();
    } else if (rc2 > 0) {
      // Parent
      park((void *)2);
      printf(1, "parent awake\n");
      printf(1, "almost done...\n");
      unpark((void *)1);
      sleep(200);
      wait();
      wait();
    }
  }
}

void test5(void) {
  printf(1, "* * * * TEST 5 * * * *\n");
  setpark((void *)3);
  unpark((void *)3);
  park((void *)3);

  // printf(1, "* * * * TEST 5 * * * *\n");
  // setpark((void *)3);

  // char *stack = malloc(4096);
  // int rc = clone(stack, 4096);
  // if (rc == 0) {
  //   // child
  //   unpark((void *)3);
  //   exit();
  // } else {
  //   // parent
  //   waitpid(rc);
  // }
  // park((void *)3);
  // exit();
  exit();
}