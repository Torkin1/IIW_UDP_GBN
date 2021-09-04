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

    // divides message in packets
    Packet **packets;
    int numOfPackets = packetize(msg, size, &packets);

    // adds the packets atomically to the battery
    pthread_mutex_lock(&(getLaunchBatteryReference() ->lock));
    if (addToLaunchBattery(packets, numOfPackets)){

        logMsg(E, "sendMessageGbn: can't add packets to the battery\n");
        pthread_mutex_unlock(&(getLaunchBatteryReference() ->lock));
        return -1;
    }
    logMsg(D, "sendMessageGbn: packets added to the battery\n");

    // registers the message in the send table
    SortingEntry *sendEntry = newSortingEntry();
    sendEntry ->sd = sd;
    sendEntry ->addr = dest_addr;
    sendEntry ->addrlen = addrlen;
    sendEntry ->errorHandler = (void (*)(int)) errorHandler;
    pthread_mutex_lock(&(getSendTableReference() ->lock));
    addToSortingTable(getSendTableReference(), packets[0] ->header ->msgId, sendEntry);
    pthread_mutex_unlock(&(getSendTableReference() ->lock));

    // now new packets in battery are safe to consume
    pthread_mutex_unlock(&(getLaunchBatteryReference() ->lock));

    // notifies launcher thread to check if new packets are in the send window
    pthread_mutex_lock(&(getSendWindowReference() ->lock));
    notifyLauncher(LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW);
    pthread_mutex_unlock(&(getSendWindowReference() ->lock));

    free(packets);
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
