#define _POSIX_C_SOURCE 199309L

#include "gbn/timer.h"
#include <stdlib.h>
#include "logger/logger.h"
#include <time.h>
#include "gbn/networkSampler.h"
#include <math.h>

// coefficients used to calculate adaptive timeout according to probability distribution of event (rtt < timeout)
typedef enum probabilityDistributionCoefficient{

    NORMAL_DISTRIBUTION = 2,
    NUM_OF_PROBABILITY_DISTRIBUTIONS
} ProbabilityDistributionCoefficient;

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

double calcAdaptiveTimeoutParam(double estimatedRttParam, double devRttParam, ProbabilityDistributionCoefficient c){

    return estimatedRttParam + ((2 * (int) c) * devRttParam);
}

void addToWait(struct timespec *timeout){

    struct timespec estimatedRtt = {0}, devRtt = {0};
    int toWait_sec, toWait_nsec;

#ifdef TIMEOUT_ADAPTIVE
    
    // gets current rtt and deviance
    pthread_mutex_lock(&networkSamplerLock);
    getEstimatedRtt(&estimatedRtt);
    getDevRtt(&devRtt);
    pthread_mutex_unlock(&networkSamplerLock);

#endif // TIMEOUT_ADAPTIVE
    
    // calcs amount of time to wait for timeout
    toWait_sec = round(calcAdaptiveTimeoutParam(estimatedRtt.tv_sec, devRtt.tv_sec, NORMAL_DISTRIBUTION));
    toWait_nsec = round(calcAdaptiveTimeoutParam(estimatedRtt.tv_nsec, devRtt.tv_nsec, NORMAL_DISTRIBUTION));
    
    // if rtt and deviance data are not available fallbacks to constant values
    if (toWait_sec == 0 && toWait_nsec == 0){
        toWait_sec = TOWAIT_CONST_SECONDS;
        toWait_nsec = TOWAIT_CONST_NANOSECONDS;
    }

    // adds time to wait to caller timeout
    timeout ->tv_sec += toWait_sec;
    timeout ->tv_nsec += (toWait_nsec != 0)? toWait_nsec : TOWAIT_CONST_NANOSECONDS;
    logMsg(D, "addToWait: timeout will ring in %d secs %d nsecs at most\n", toWait_sec, toWait_nsec);

}

void calcTimeout(struct timespec *timeout){

    clock_gettime(CLOCK_REALTIME, timeout);
    addToWait(timeout);
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