#include <stdlib.h>
#include "gbn/gbn.h"
#include "gbn/launchBattery.h"
#include "logger/logger.h"
#include "gbn/launcher.h"
#include <errno.h>
#include <string.h>
#include "gbn/sendWindow.h"

LaunchPad *newLaunchPad(){

    return calloc(1, sizeof(LaunchPad));

}

LaunchBattery *newLaunchBattery(){

    LaunchBattery *launchBattery = calloc(1, sizeof(LaunchBattery));

    for (int i = 0; i < QUEUE_LEN; i ++){
        (launchBattery -> battery)[i] = newLaunchPad();
    }

    launchBattery->contiguousPadsAvailable = QUEUE_LEN;

    pthread_mutex_init(&(launchBattery -> lock), NULL);

    return launchBattery;
}

// caution: destroy the battery only when you are sure that no threads are willing to use it anymore
int destroyLaunchBattery(LaunchBattery *self){

    if (pthread_mutex_destroy(&(self ->lock))){
        int err = errno;
        logMsg(E, "destroyLaunchBattery: can't destroy lock on battery: %s\n", strerror(err));
        return -1;
    }
    for (int i = 0; i < QUEUE_LEN; i ++){
        free(self -> battery[i]);
    }
    free(self);

    return 0;

}

void destroyLaunchPad(LaunchPad *self){

   free(self ->packet);
   free(self);

}

// singleton launchBattery
// pthread_once is used for synchronizing multiple threads calling getReference at the same time
static LaunchBattery *launchBattery;
static pthread_once_t isLaunchBatteryInitialized = PTHREAD_ONCE_INIT;

void initLaunchBattery(){

    launchBattery = newLaunchBattery();

}

LaunchBattery *getLaunchBatteryReference(){

    if (launchBattery == NULL){

        // synchronized
        pthread_once(&isLaunchBatteryInitialized, initLaunchBattery);
    }

    return launchBattery;

}

bool isLaunchPadAvailable(LaunchPad *self){

    switch (self ->status){

        case EMPTY:
        case ACKED:
        case LOST:
            return true;
        default:
            return false;
    }

}

void resetLaunchPad(LaunchPad *self){

    destroyPacket(self ->packet);
    memset(self, 0, sizeof(LaunchPad));

}

bool willTheyFit(int n){

    return (n <= getLaunchBatteryReference() ->contiguousPadsAvailable)? true : false;

}

int updateContiguousPads(int change){

    getLaunchBatteryReference() ->contiguousPadsAvailable += change;
    return getLaunchBatteryReference() ->contiguousPadsAvailable;

}

// adds packets to launchbattery atomically
int addToLaunchBatteryAtomically(Packet *packets[], int n){

    logMsg(D, "about to add %d packets to battery\n", n);

    int err;
    if ((err = pthread_mutex_lock(&(getLaunchBatteryReference() ->lock)))){

        logMsg(E, "addToLaunchBattery: unable to lock battery: %s\n", strerror(err));
        return -1;
    }

    // if there is not enough contiguous space, the packets will not be added
    if(!willTheyFit(n)){

        logMsg(E, "addToLaunchBattery: %d packets to add, but only %d pads are available", n, getLaunchBatteryReference() ->contiguousPadsAvailable);
        return -1;
    }

    // adds the packets to the queue.
    int start = getLaunchBatteryReference() ->nextAvailableIndex;
    for (int i = 0; i < n; i ++){

        LaunchPad *currentLaunchPad = getLaunchBatteryReference() ->battery[(start + i) % QUEUE_LEN];
        if (isLaunchPadAvailable(currentLaunchPad)){

            // Launch pads containing ACKED or LOST packets are cleared before use
            if(currentLaunchPad ->status == ACKED || currentLaunchPad ->status == LOST){

                resetLaunchPad(currentLaunchPad);
            }
            packets[i] ->header ->index = start + i;
            packets[i] ->header ->endIndex = start + n - 1;
            currentLaunchPad ->packet = packets[i];
            currentLaunchPad ->status = READY;
            updateContiguousPads(-1);
        }
    }
    getLaunchBatteryReference() ->nextAvailableIndex = (start + n) % QUEUE_LEN;

    if ((err = pthread_mutex_unlock(&(getLaunchBatteryReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to unlock battery: %s\n", strerror(err));
        return -1;
    }

    // notifies launcher thread if new packets are in the send window
    if ((err = pthread_mutex_lock(&(getSendWindowReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to lock sendWindow: %s\n", strerror(err));
        return -1;
    }

    int currentBase, currentNextSeqNum;
    currentBase = getSendWindowReference() ->base;
    currentNextSeqNum = getSendWindowReference() ->nextSeqNum;

    /*

        since the battery has a cyclic structure, we can have two situations:

                   b           n
        1)    [x] [ ] [ ] [ ] [ ]     nextSeqNum > base, x is not in window

                   n           b
        2)    [x] [ ] [ ] [ ] [ ]     nextSeqNum < base, x is in window

        isInWindow is set accordingly to whichever is the highest among base and nextSeqNum

    */

    bool isInWindow;

    if (currentBase < currentNextSeqNum){
        isInWindow = (start >= currentBase && start < currentNextSeqNum);
    }
    else {
        isInWindow = (start >= currentBase && start < QUEUE_LEN) && (start >= 0 && start < currentNextSeqNum);
    }

    if (isInWindow){
        if (notifyLauncher(NEW_PACKETS_IN_SEND_WINDOW, NULL)){
            logMsg(E, "couldn't notify the launcher\n");
            return -1;
        }
        logMsg(D, "launcher notified about event %d\n", NEW_PACKETS_IN_SEND_WINDOW);
    }

    if ((err = pthread_mutex_unlock(&(getSendWindowReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to unlock send window: %s\n", strerror(err));
        return -1;
    }

    return 0;

}

