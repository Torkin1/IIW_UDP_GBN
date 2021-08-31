#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <pthread.h>

// window used to choose which packets are to be sent from the queue
typedef struct sendWindow{

    int base;               // last non-acked packet index in window
    int nextSeqNum;         // Index of first READY packet next to the last SENT packet in window
    pthread_mutex_t lock;

} SendWindow;


void destroySendWindow(SendWindow *self);
SendWindow *newSendWindow();

int getWinSize();
int calcAdaptiveWinSize();
int updateAdaptiveWinSize(float rtt);

#endif // WINDOW_H_INCLUDED