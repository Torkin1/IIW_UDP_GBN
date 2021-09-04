#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gbn/networkSampler.h"

#define ALPHA 0.125 // 1/8
#define BETA 0.25   // 1/4

typedef struct networkParams {

    struct timespec estimatedRtt;               // Round Trip Time
    struct timespec devRtt;                     // deviance of sampleRtt from estimatedRtt
    struct timespec estimatedUploadTime;        // L/R

} NetworkParams;

static NetworkParams *networkParams = NULL;
static pthread_once_t isNetworkParamsInitialized = PTHREAD_ONCE_INIT;
pthread_mutex_t networkSamplerLock = PTHREAD_MUTEX_INITIALIZER;

NetworkParams *newNetworkParams(){

    NetworkParams *params = calloc(1, sizeof(NetworkParams));
    return params;

}

void destroyNetworkParams(NetworkParams *self){

    free(self);

}

void destroyNetworkSampler(){

    destroyNetworkParams(networkParams);
}

void initNetworkParams(){

    networkParams = newNetworkParams();
}

NetworkParams *getNetworkParamsReference(){

    if (networkParams == NULL){

        pthread_once(&isNetworkParamsInitialized, initNetworkParams);
    }

    return networkParams;

}

double calcEWMA(double estimated, double sample, double weightDecreaseDegree){

    return ((1 - weightDecreaseDegree) * estimated) + (weightDecreaseDegree * sample);

}

double calcSampleDeviance(int estimated, int sample){

    return abs(sample - estimated);
}

void updateWithEWMA(struct timespec *toUpdate, struct timespec sample, double weightDecreaseDegree){

    toUpdate ->tv_sec = round(calcEWMA(toUpdate ->tv_sec, sample.tv_sec, weightDecreaseDegree));
    toUpdate ->tv_nsec = round(calcEWMA(toUpdate ->tv_nsec, sample.tv_nsec, weightDecreaseDegree));
}

void updateEstimatedRtt(struct timespec sampleRtt){

    // current implementation uses EWMA to update rtt and deviance
    
    // updates estimated rtt
    updateWithEWMA(&(getNetworkParamsReference() ->estimatedRtt), sampleRtt, ALPHA);

    // calcs deviance and updates devRtt
    struct timespec devRttSample = {0};
    devRttSample.tv_sec = calcSampleDeviance(sampleRtt.tv_sec, getNetworkParamsReference() ->estimatedRtt.tv_sec);
    devRttSample.tv_nsec = calcSampleDeviance(sampleRtt.tv_nsec, getNetworkParamsReference() ->estimatedRtt.tv_nsec);
    updateWithEWMA(&(getNetworkParamsReference() ->devRtt), devRttSample, BETA);

    logMsg(
        D,
        "updateEstimatedRtt: estimated RTT is of %d secs %d nsecs, with a deviance of %d secs %d nsecs\n",
        getNetworkParamsReference() ->estimatedRtt.tv_sec,
        getNetworkParamsReference() ->estimatedRtt.tv_nsec,
        getNetworkParamsReference() ->devRtt.tv_sec,
        getNetworkParamsReference() ->devRtt.tv_nsec
        );


}

void getEstimatedRtt(struct timespec *estimatedRttBuf){

    estimatedRttBuf ->tv_sec = getNetworkParamsReference() ->estimatedRtt.tv_sec;
    estimatedRttBuf ->tv_nsec = getNetworkParamsReference() ->estimatedRtt.tv_nsec;
}

void getDevRtt(struct timespec *devRttbuf){

    devRttbuf ->tv_sec = getNetworkParamsReference() ->devRtt.tv_sec;
    devRttbuf ->tv_nsec = getNetworkParamsReference() ->devRtt.tv_nsec;
}