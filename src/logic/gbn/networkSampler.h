#ifndef NETWORKSAMPLER_H_INCLUDED
#define NETWORKSAMPLER_H_INCLUDED

#include <pthread.h>

typedef struct networkParams {

    pthread_mutex_t lock;
    int rtt;            // Round Trip Time
    int uploadTime;     // L/R

} NetworkParams;

NetworkParams *getNetworkParamsReference();
void destroyNetworkParams(NetworkParams *self);

#endif // NETWORKSAMPLER_H_INCLUDED