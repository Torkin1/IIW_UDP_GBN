#include "sendWindow.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>
#include "gbn/launchBattery.h"

static int winsize;

// singleton send window
static SendWindow *sendWindow;
static pthread_once_t isSendWindowInitialized = PTHREAD_ONCE_INIT;

SendWindow *newSendWindow(){

    SendWindow *sendWindow = calloc(1, sizeof(SendWindow));
    pthread_mutex_init(&(sendWindow ->lock), NULL);

    return sendWindow;
}

void destroySendWindow(SendWindow *self){

    int err;

    if ((err = pthread_mutex_destroy(&(self ->lock)))){
        logMsg(W, "destroySendWindow: can't destroy lock: %s\n", strerror(err));
    }
    free(self);

}

int calcAdaptiveWinSize(){

    // TODO: actually calculate winsize
    int res = 2;

    return res;

}

void initSendWindow(){

    sendWindow = newSendWindow();

    // sets up initial values for sendWindow according to winsize
    winsize = calcAdaptiveWinSize();
    sendWindow ->nextSeqNum = winsize;

}

SendWindow *getSendWindowReference(){

    if (sendWindow == NULL){

        pthread_once(&isSendWindowInitialized, initSendWindow);
    }

    return sendWindow;

}

bool isInWindow(int i){

    /*

        since the battery has a cyclic structure, we can have two situations:

                   b           n
        1)    [x] [ ] [ ] [ ] [ ]     nextSeqNum > base, x is not in window

                   n           b
        2)    [x] [ ] [ ] [ ] [ ]     nextSeqNum < base, x is in window

        isInWindow is set accordingly to whichever is the highest among base and nextSeqNum

    */

    bool isInWindow;
    int currentBase = getSendWindowReference() ->base;
    int currentNextSeqNum = getSendWindowReference() ->nextSeqNum;

    if (currentBase < currentNextSeqNum){
        isInWindow = (i >= currentBase && i < currentNextSeqNum);
    }
    else {
        isInWindow = (i >= currentBase && i < QUEUE_LEN) && (i >= 0 && i < currentNextSeqNum);
    }

    return isInWindow;

}

