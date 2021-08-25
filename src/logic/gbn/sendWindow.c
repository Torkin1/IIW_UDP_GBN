#include "sendWindow.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

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
    int res = 10;

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
