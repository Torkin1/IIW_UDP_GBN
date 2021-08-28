#include "gbn/networkSampler.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

static NetworkParams *networkParams;
static pthread_once_t isNetworkParamsInitialized = PTHREAD_ONCE_INIT;

NetworkParams *newNetworkParams(){

    NetworkParams *params = calloc(1, sizeof(NetworkParams));
    pthread_mutex_init(&(params ->lock), NULL);

    return params;

}

void destroyNetworkParams(NetworkParams *self){

    int err;

    if ((err = pthread_mutex_destroy(&(self ->lock)))){
        logMsg(W, "destroyNetworkParams: can't destroy lock: %s\n", strerror(err));
    }
    free(self);

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

void updateRTT(float sampleRTT){

    // TODO:

}

void updateUploadTime(float uploadTime){

    // TODO:

}
