#define _POSIX_C_SOURCE 199309L

#include "testSuites.h"
#include "gbn/timer.h"
#include <time.h>
#include "logger/logger.h"
#include <pthread.h>
#include <stdio.h>

static struct timespec before, after;

void doRing(){

    clock_gettime(CLOCK_REALTIME, &after);
}

void testTimerConst(){

    char buf[1024];
    clock_gettime(CLOCK_REALTIME, &before);
    logMsg(D, "before set %d sec and %d nanosec\n", before.tv_sec, before.tv_nsec);
    Timer *timer = newTimer();
    logMsg(D, "timer object created\n");
    startTimer(timer, AT_TIMEOUT_RING_THEN_SHUTDOWN, doRing);
    pthread_join(timer ->timerTid, NULL);
    logMsg(D, "doRing: after set %d sec %d nanosec\n", after.tv_sec, after.tv_nsec);
    int elapsedSecs = after.tv_sec - before.tv_sec;
    int expected = TOWAIT_CONST_SECONDS;
    sprintf(buf, "expected: %d, actual %d\n", expected, elapsedSecs);
    assertEquals(&expected, &elapsedSecs, sizeof(int), buf, NULL);
    destroyTimer(timer);

}