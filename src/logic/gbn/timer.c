#define _POSIX_C_SOURCE 199309L

#include "gbn/timer.h"
#include <stdlib.h>
#include "logger/logger.h"
#include <time.h>

Timer *newTimer(){

    Timer *timer = calloc(1, sizeof(Timer));
    pthread_mutex_init(&(timer ->isAliveLock), NULL);
    pthread_mutex_init(&(timer ->atTimeoutLock), NULL);
    pthread_cond_init(&(timer ->atTimeoutCond), NULL);
    pthread_cond_init(&timer ->isAliveCond, NULL);

    return timer;

}

void destroyTimer(Timer *self){
    
    pthread_mutex_destroy(&(self ->isAliveLock));
    pthread_mutex_destroy(&(self ->atTimeoutLock));
    pthread_cond_destroy(&(self ->atTimeoutCond));
    pthread_cond_destroy(&self ->isAliveCond);
    free(self);
}

void calcConstTimeout(struct timespec *timeout){

    timeout ->tv_sec += TOWAIT_CONST_SECONDS;
    timeout ->tv_nsec += TOWAIT_CONST_NANOSECONDS;    

}

void calcAdaptiveTimeout(struct timespec *timeout){

    // TODO: calc adaptive time to wait and add it to timeout
}

void calcTimeout(struct timespec *timeout){

    clock_gettime(CLOCK_REALTIME, timeout);

    #ifdef TIMEOUT_CONST
    
        calcConstTimeout(timeout);
    #else

        calcAdaptiveTimeout(timeout);

    #endif // TIMEOUT_CONST

}

void *timer(void *args){

    // initial setup
    Timer *self = (Timer *) args;
    pthread_mutex_lock(&(self ->isAliveLock));
    self -> isAlive = true;
    pthread_mutex_unlock(&(self ->isAliveLock));
    pthread_cond_signal(&(self ->isAliveCond));

    struct timespec timeout;

    while(self ->isAlive){

        pthread_mutex_lock(&(self ->atTimeoutLock));
        calcTimeout(&timeout);
        logMsg(D, "timer: about to start wait\n");
        pthread_cond_timedwait(&(self ->atTimeoutCond), &(self ->atTimeoutLock), &timeout);
        logMsg(D, "timer: reached timeout\n");
        switch (self ->atTimeout)
        {
        
        case AT_TIMEOUT_RING_THEN_SHUTDOWN:
            if (self -> ring != NULL){
                logMsg(D, "timer: ring!\n");
                (self ->ring)();
            }
                
        case AT_TIMEOUT_SHUTDOWN:
            logMsg(D, "timer: about to shutdown\n");
            self ->isAlive = false;
            break;
        
        case AT_TIMEOUT_RING_THEN_RESTART:
            if (self -> ring != NULL){
                logMsg(D, "timer: ring!\n");
                (self ->ring)();
            }
        
        case AT_TIMEOUT_RESTART:
            self ->atTimeout = AT_TIMEOUT_RING_THEN_RESTART;
            logMsg(D, "timer: about to restart\n");
            break;
        default:
            break;
        }

        pthread_mutex_unlock(&(self ->atTimeoutLock));
    }

    logMsg(D, "timer: I'm dying lol\n");

    return NULL;
    

}

int startTimer(Timer *self, AtTimeout atTimeout, void (*ring) (void)){

    // won't start if it's already alive
    if (self ->isAlive){
        logMsg(E, "startTimer: timer %d is already started\n", self ->timerTid);
        return -1;
    }

    self ->ring = ring;

    pthread_mutex_lock(&self ->atTimeoutLock);
    if (atTimeout >= AT_TIMEOUT_RING_THEN_RESTART){
        self ->atTimeout = atTimeout;
    }
    pthread_mutex_unlock(&self ->atTimeoutLock);

    // launches timer thread and waits for it to succesfully start
    pthread_mutex_lock(&(self ->isAliveLock));
    
    pthread_create(&(self ->timerTid), NULL, timer, (void *) self);
    while(!self ->isAlive){
        pthread_cond_wait(&(self ->isAliveCond), &(self ->isAliveLock));
    }
    pthread_mutex_unlock(&(self ->isAliveLock));
    logMsg(D, "startTimer: timer %d started\n", self ->timerTid);

    
    return 0;
}

void timeout(Timer *self, AtTimeout atTimeout){
    
    logMsg(D, "timeout: alarm will go off with atTimeout=%d\n", atTimeout);
    
    if (atTimeout >= AT_TIMEOUT_RING_THEN_RESTART){
        pthread_mutex_lock(&self ->atTimeoutLock);
        self -> atTimeout = atTimeout;
        pthread_mutex_unlock(&self ->atTimeoutLock);
    }

    pthread_cond_signal(&self ->atTimeoutCond);



}