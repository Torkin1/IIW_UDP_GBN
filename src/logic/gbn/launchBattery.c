#include <stdlib.h>
#include "gbn/gbn.h"
#include "gbn/launchBattery.h"
#include "logger/logger.h"
#include "gbn/launcher.h"
#include <errno.h>
#include <string.h>

// singleton send window
static SendWindow *sendWindow;
static pthread_once_t isSendWindowInitialized = PTHREAD_ONCE_INIT;

// singleton sorting table
static SortingTable *sendTable;
static pthread_once_t isSendTableInitialized = PTHREAD_ONCE_INIT;


void initSendWindow(){

    sendWindow = newSendWindow();
}

SendWindow *getSendWindowReference(){

    if (sendWindow == NULL){

        pthread_once(&isSendWindowInitialized, initSendWindow);
    }

    return sendWindow;

}

void initSendTable(){

    sendTable = newSortingTable();

}

SortingTable *getSendTableReference(){

    if (sendTable == NULL){

        pthread_once(&isSendTableInitialized, initSendTable);
    }

    return sendTable;

}


bool isInWindow(int i){

    /*

        since the battery has a cyclic structure, we can have two situations:

                   b           n
        1)    [x] [ ] [ ] [ ] [ ]     nextSeqNum > base, x is not in window

                   n           b
        2)    [x] [ ] [ ] [ ] [ ]     nextSeqNum < base, x is in window

        isInWindow is set accordingly to whichever is the highest among base and nextSeqNum

    */

    bool isInWindow;
    int currentBase = getSendWindowReference() ->base;
    int currentNextSeqNum = getSendWindowReference() ->nextSeqNum;

    if (currentBase < currentNextSeqNum){
        isInWindow = (i >= currentBase && i < currentNextSeqNum);
    }
    else {
        isInWindow = (i >= currentBase && i < QUEUE_LEN) || (i >= 0 && i < currentNextSeqNum);
    }

    return isInWindow;

}

// launch battery

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

    logMsg(D, "addToLaunchBattery: about to add %d packets to battery\n", n);

    int err;
    if ((err = pthread_mutex_lock(&(getLaunchBatteryReference() ->lock)))){

        logMsg(E, "addToLaunchBattery: unable to lock battery: %s\n", strerror(err));
        return -1;
    }

    // if there is not enough contiguous space, the packets will not be added
    if(!willTheyFit(n)){

        logMsg(E, "addToLaunchBattery: %d packets to add, but only %d pads are available\n", n, getLaunchBatteryReference() ->contiguousPadsAvailable);
        return -1;
    }

    // adds the packets to the queue.
    int start = getLaunchBatteryReference() ->nextAvailableIndex;
    int atIndex, atEndIndex, atNextAvailableIndex;;
    for (int i = 0; i < n; i ++){

        atIndex = (start + i) % QUEUE_LEN;
        atEndIndex = (start + n - 1) % QUEUE_LEN;
        atNextAvailableIndex = (start + n) % QUEUE_LEN;
        LaunchPad *currentLaunchPad = getLaunchBatteryReference() ->battery[atIndex];
        if (isLaunchPadAvailable(currentLaunchPad)){

            // Launch pads containing ACKED or LOST packets are cleared before use
            if(currentLaunchPad ->status == ACKED || currentLaunchPad ->status == LOST){

                resetLaunchPad(currentLaunchPad);
            }
            packets[i] ->header ->index = atIndex;
            packets[i] ->header ->endIndex = atEndIndex;
            packets[i] ->header ->queueLen = QUEUE_LEN;
            currentLaunchPad ->packet = packets[i];
            currentLaunchPad ->status = READY;
            updateContiguousPads(-1);
        }
    }
    getLaunchBatteryReference() ->nextAvailableIndex = atNextAvailableIndex;

    if ((err = pthread_mutex_unlock(&(getLaunchBatteryReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to unlock battery: %s\n", strerror(err));
        return -1;
    }

    logMsg(D, "addToLaunchBattery: packets added at index %d\n", start);

    // notifies launcher thread if new packets are in the send window
    if ((err = pthread_mutex_lock(&(getSendWindowReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to lock sendWindow: %s\n", strerror(err));
        return -1;
    }

    if (isInWindow(start)){
        if (notifyLauncher(LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW)){
            logMsg(E, "addToLaunchBattery: couldn't notify the launcher\n");
            return -1;
        }
        logMsg(D, "addToLaunchBattery: launcher notified about event %d\n", LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW);
    }

    if ((err = pthread_mutex_unlock(&(getSendWindowReference() ->lock)))){
        logMsg(E, "addToLaunchBattery: unable to unlock send window: %s\n", strerror(err));
        return -1;
    }

    return 0;

}

