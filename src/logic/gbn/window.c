#include "gbn/window.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

#define WINSIZE_DEFAULT 2

static int winsize;

SendWindow *newSendWindow(){

    SendWindow *sendWindow = calloc(1, sizeof(SendWindow));
    winsize = WINSIZE_DEFAULT;
    sendWindow ->nextSeqNum = winsize;

    return sendWindow;
}

void destroySendWindow(SendWindow *self){

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