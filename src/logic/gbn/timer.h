#include <pthread.h>
#include <stdbool.h>

#ifdef TIMEOUT_CONST
    #define TOWAIT_CONST_SECONDS 3;          
    #define TOWAIT_CONST_NANOSECONDS 0;
#endif // TIMEOUT_CONST

// Describes what the timer has to do when it timeouts
typedef enum atTimeout{

    AT_TIMEOUT_RING_THEN_RESTART,          // timer shall ring at timeout and restart
    AT_TIMEOUT_RING_THEN_SHUTDOWN,          // timer shall ring at timeout, will flag itself as not alive and exit
    AT_TIMEOUT_RESTART,                    // timer shall not ring and will immediatly restart with atTimeout = AT_TIMEOUT_RING_THEN_RESTART  
    AT_TIMEOUT_SHUTDOWN,                   // timer shall not ring, will flag itself as not alive and exit

    NUM_OF_AT_TIMEOUT

} AtTimeout;

// Timer variables
typedef struct timer{

    pthread_t timerTid;                 // tid of timer thread
    void (*ring)(void);                 // what the timer has to do when rings 
    bool isAlive;                       // true if timer thread is alive
    AtTimeout atTimeout;                // what the timer has to do at timeout
    pthread_mutex_t isAliveLock;
    pthread_mutex_t atTimeoutLock;
    pthread_cond_t atTimeoutCond;  
    pthread_cond_t isAliveCond;        

} Timer;

/* 
@return a pointer to a new Timer object
*/
Timer *newTimer();

// @param self a pointer to the Timer object to destroy
void destroyTimer(Timer *self);

/*
launches the timer thread with specified timeout action
@param self pointer to timer target
@param atTimeout what the timer has to do when it timeouts. <= 0 defaults to AT_TIMEOUT_RING_THEN_RESTART
@param ring function called when the timer rings. Can be NULL 
@return 0 if success, else -1
*/
int startTimer(Timer *self, AtTimeout atTimeout, void (*ring) (void));

/*
Changes the timeout action and makes the timer go in timeout
@param self pointer to Timer target
@param atTimeout what the timer has to do when it timeouts. < 0 if you don't want to change it
*/
void timeout(Timer *self, AtTimeout atTimeout); 


