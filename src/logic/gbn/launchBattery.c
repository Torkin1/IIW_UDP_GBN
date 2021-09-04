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
pthread_mutex_t sendWindowLock = PTHREAD_MUTEX_INITIALIZER;

// singleton sorting table
static SortingTable *sendTable;
static pthread_once_t isSendTableInitialized = PTHREAD_ONCE_INIT;
pthread_mutex_t sendTableLock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t launchBatteryLock = PTHREAD_MUTEX_INITIALIZER;


void initSendWindow(){

    logMsg(D, "initSendWindow: new send window requested\n");
    sendWindow = newSendWindow();
    logMsg(D, "initSendWindow: send window created\n");
}

SendWindow *getSendWindowReference(){
    
    logMsg(D, "getSendWindowReference: requested window reference\n");
    if (sendWindow == NULL){

        pthread_once(&isSendWindowInitialized, initSendWindow);
        logMsg(D, "getSendWindowReference: pthread_once returned\n");
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

    return launchBattery;
}

// caution: destroy the battery only when you are sure that no threads are willing to use it anymore
int destroyLaunchBattery(LaunchBattery *self){

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
    
    // triggers initialization of other components needed by launchbattery to work. This is needed because other components use signal-unsafe functions
    getSendTableReference();
    getSendWindowReference();
    getLauncherId(NULL);

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
int addToLaunchBattery(Packet *packets[], int n){

    logMsg(D, "addToLaunchBattery: about to add %d packets to battery\n", n);

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
    logMsg(D, "addToLaunchBattery: packets added at index %d\n", start);
    
    return 0;

}

