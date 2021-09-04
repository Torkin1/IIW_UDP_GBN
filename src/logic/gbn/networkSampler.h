#ifndef NETWORKSAMPLER_H_INCLUDED
#define NETWORKSAMPLER_H_INCLUDED

#include <pthread.h>

extern pthread_mutex_t networkSamplerLock;       // lock on network sampler ref

// @param sampleRtt value of sampled rtt
void updateEstimatedRtt(struct timespec sampleRtt);

// @param estimatedRttBuf pointer to an already initialized buffer which will hold current value of rtt
void getEstimatedRtt(struct timespec *estimatedRttBuf);

// @param devRttBuf pointer to an already initialized buffer which will hold current value of devRtt
void getDevRtt(struct timespec *devRttBuf);

void destroyNetworkSampler();


#endif // NETWORKSAMPLER_H_INCLUDED
