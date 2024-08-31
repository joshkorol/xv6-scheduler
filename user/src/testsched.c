#include "types.h"
#include "stat.h"
#include "user.h"
#include "sched.h"

void test0(void);
void test1(void);
void test2(void);
void test3(void);
void test4(void);

int main(int argc, char *argv[]) {
  if (argc > 0) {
    printf(1, "argc: %d, argv[1]: %c\n", argc, argv[1]);
    switch(atoi(argv[1])){
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
        default:
            test0();
            test1();
            test2();
            test3();
            break;
    }
  }
  exit();
}

void test0(void) {
    printf(1, "* * * * TEST 0 * * * *\n");
    int a = fork();
    if (a > 0) {
        sleep(5);
        for (int i = 0; i < 50; ++i) {
            printf(1, "i: %d, p: %d\n", i, getpid());
        }
        wait();
    } else { // Child code
        sleep(5);
        for (int j = 0; j < 50; j++) {
            printf(1, "j: %d, c: %d\n", j, getpid());
        }
    }
    exit();
}

// FIFO RR
void test1(void) {
    printf(1, "* * * * TEST 1 * * * *\n");
    int a = fork();
    if (a > 0) {
        int result = setscheduler(getpid(), SCHED_FIFO, 10);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_FIFO, 10);
        setscheduler(getpid(), SCHED_FIFO, 10);
        sleep(5);
        for (int i = 0; i < 50; ++i) {
            printf(1, "i: %d, p: %d\n", i, getpid());
        }
        wait();
    } else { // Child code
        int result = setscheduler(getpid(), SCHED_FIFO, 9);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_FIFO, 9);
        sleep(5);
        for (int j = 0; j < 50; j++) {
            printf(1, "j: %d, c: %d\n", j, getpid());
        }
    }
    exit();
}

void test2(void) {
    printf(1, "* * * * TEST 2 * * * *\n");
    int a = fork();
    if (a > 0) {
        int result = setscheduler(getpid(), SCHED_FIFO, 10);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_FIFO, 10);
        setscheduler(getpid(), SCHED_FIFO, 10);
        sleep(10);
        for (int i = 0; i < 50; ++i) {
            printf(1, "i: %d, p: %d\n", i, getpid());
        }
        wait();
    } else { // Child code
        int result = setscheduler(getpid(), SCHED_FIFO, 10);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_FIFO, 10);
        sleep(10);
        for (int j = 0; j < 50; j++) {
            printf(1, "j: %d, c: %d\n", j, getpid());
        }
    }
    exit();
}

// FIFO before RR
void test3(void) {
    printf(1, "* * * * TEST 3 * * * *\n");
    int a = fork();
    if (a > 0) {
        //int result = setscheduler(getpid(), SCHED_FIFO, 0);
        int result = setscheduler(getpid(), SCHED_RR, 10);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_RR, 10);
        sleep(10);
        for (int i = 0; i < 100; ++i) {
            printf(1, "i: %d, p: %d\n", i, getpid());
            if (i == 25) {
                int result = setscheduler(getpid(), SCHED_FIFO, 0);
                printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_FIFO, 0);
            }
        }
        wait();
    } else { // Child code
        int result = setscheduler(getpid(), SCHED_RR, 10);
        printf(1, "result: %d, setscheduler(%d, %d, %d)\n", result, getpid(), SCHED_RR, 10);
        sleep(10);
        for (int j = 0; j < 100; j++) {
            printf(1, "j: %d, c: %d\n", j, getpid());
        }
    }
    exit();
}