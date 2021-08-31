#include "gbn/window.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

#define WINSIZE_DEFAULT 2

static int winsize;

SendWindow *newSendWindow(){

    SendWindow *sendWindow = calloc(1, sizeof(SendWindow));
    pthread_mutex_init(&(sendWindow ->lock), NULL);
    winsize = WINSIZE_DEFAULT;
    sendWindow ->nextSeqNum = winsize;

    return sendWindow;
}

void destroySendWindow(SendWindow *self){

    int err;

    if ((err = pthread_mutex_destroy(&(self ->lock)))){
        logMsg(W, "destroySendWindow: can't destroy lock: %s\n", strerror(err));
    }
    free(self);

}

int getWinSize(){

    return winsize;

}

int calcAdaptiveWinSize(){

    // TODO: actually calculate winsize
    int res = -1;

    return res;

}

int updateAdaptiveWinSize(float rtt){

    int old = getWinSize();

    // TODO:

    return old;

}