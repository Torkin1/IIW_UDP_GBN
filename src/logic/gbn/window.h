#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <pthread.h>

// window used to choose which packets are to be sent from the queue
typedef struct sendWindow{

    int base;               // last non-acked packet index in window
    int nextSeqNum;         // Index of first READY packet next to the last SENT packet in window

} SendWindow;


void destroySendWindow(SendWindow *self);
SendWindow *newSendWindow();

// @return max num of packets that can be in the window at the same time
int getWinSize();

#endif // WINDOW_H_INCLUDED