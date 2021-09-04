#define _DEFAULT_SOURCE
#include "logger/logger.h"
#include "gbn/gbn.h"
#include "gbn/packet.h"
#include "gbn/launchBattery.h"
#include "gbn/launcher.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "gbn/catcher.h"
#include <pthread.h>

int sendMessageGbn(int sd, struct sockaddr *dest_addr, socklen_t addrlen, void *msg, int size, void (*errorHandler)(SendError)){

    int err;
    
    // divides message in packets
    Packet **packets;
    int numOfPackets = packetize(msg, size, &packets);

    // adds the packets atomically to the battery
    pthread_mutex_lock(&launchBatteryLock);
    logMsg(D, "sendMessageGbn: acquired lock on battery\n");
    if (addToLaunchBattery(packets, numOfPackets)){

        logMsg(E, "sendMessageGbn: can't add packets to the battery\n");
        err = pthread_mutex_unlock(&launchBatteryLock);
        logMsg(E, "unlock1: %s\n", strerror(err));
        return -1;
    }
    logMsg(D, "sendMessageGbn: packets added to the battery\n");

    // registers the message in the send table
    SortingEntry *sendEntry = newSortingEntry();
    sendEntry ->sd = sd;
    sendEntry ->addr = dest_addr;
    sendEntry ->addrlen = addrlen;
    sendEntry ->errorHandler = (void (*)(int)) errorHandler;
    pthread_mutex_lock(&sendTableLock);
    logMsg(D, "sendMessageGbn: acquired lock on send table\n");
    addToSortingTable(getSendTableReference(), packets[0] ->header ->msgId, sendEntry);
    err = pthread_mutex_unlock(&sendTableLock);
    logMsg(D, "sendMessageGbn: released lock on send table\n");
    logMsg(E, "unlock2: %s\n", strerror(err));

    // now new packets in battery are safe to consume
    err = pthread_mutex_unlock(&launchBatteryLock);
    logMsg(E, "unlock3: %s\n", strerror(err));
    logMsg(D, "sendMessageGbn: released lock on battery\n");

    // notifies launcher thread to check if new packets are in the send window
    notifyLauncher(LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW);

    free(packets);
    logMsg(D, "sendMessageGbn: done\n");
    return 0;

}

/*  Returns when a data message is available.
    Pointer to dynamically allocated message will be stored in *msg and its size in *size.
    Sender addr and its size will be stored in *sender_addr and *addrlen;
    Socket must be already bound to an addr before passing it to this function.
*/ 
int recvMessageGbn(int sd, struct sockaddr *senderAddr, socklen_t *senderAddrlen, void **msgBuf, int *size){

    return listenForData(sd, senderAddr, senderAddrlen, msgBuf, size);
}
