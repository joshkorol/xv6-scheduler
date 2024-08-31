#include "types.h"
#include "stat.h"
#include "user.h"
#include "threads.h"
#include "sched.h"

#define DEBUG(code) printf(1, "%d\n", code);

void test0(void);
void test1(void);
void test2(void);
void test3(void);
void test4(void);
void test5(void);

void *chan1 = (void *)1;
void *chan2 = (void *)2;
void *chan3 = (void *)3;

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
  DEBUG(setpark(chan1));
  DEBUG(unpark(chan1));
  DEBUG(park(chan1));
}

void test1(void) {
  printf(1, "* * * * TEST 1 * * * *\n");
  DEBUG(setpark(chan2));
  char *stack = malloc(4096);
  int rc = clone(stack, 4096);
  if (rc == 0) {
    // child
    DEBUG(unpark(chan2));
    exit();
  } else {
    // parent
    waitpid(rc);
  }
  DEBUG(park(chan2));
  exit();
}

void test2(void) {
  printf(1, "* * * * TEST 2 * * * *\n");
  DEBUG(setpark(chan1));
  DEBUG(unpark(chan2));
  DEBUG(park(chan1));
}

void test3(void) {
  printf(1, "* * * * TEST 3 * * * *\n");
  DEBUG(setpark(chan2));
  char *stack = malloc(4096);
  int rc = clone(stack, 4096);
  if (rc == 0) {
    // child
    DEBUG(unpark(chan2));
    exit();
  } else {
    // parent
    waitpid(rc);
  }
  DEBUG(park(chan2));
  exit();
}

void test4(void) {
  printf(1, "* * * * TEST 4 * * * *\n");
  char *stack = malloc(4096);
  int rc = clone(stack, 4096);
  if (rc == 0) {
    DEBUG(setpark(chan1));
    DEBUG(unpark(chan1));
    // intended chahnnel = 1
    // unpark_after_set = 1
    park(chan2);
    /////////// sleep()
    printf(1, "Going to sleep\n");
    setpark(chan1);
    park(chan1);
    exit();
  } else {
    // parent
    sleep(100);
    unpark(chan2);
    waitpid(rc);
  }
  exit();
}

void test5(void) {
  printf(1, "* * * * TEST 5 * * * *\n");
  DEBUG(setpark(chan3));
  exit();
}