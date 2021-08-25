#ifndef SENDWINDOW_H_INCLUDED
#define SENDWINDOW_H_INCLUDED

#include <pthread.h>

// window used to choose which packets are to be sent from the queue
typedef struct sendWindow{

    int base;               // last non-acked packet index in window
    int nextSeqNum;         // Index of first READY packet next to the last SENT packet in window
    pthread_mutex_t lock;

} SendWindow;

SendWindow *getSendWindowReference();
void destroySendWindow(SendWindow *self);


#endif // SENDWINDOW_H_INCLUDED
