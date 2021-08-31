#include "logger/logger.h"
#include "gbn/gbn.h"
#include "gbn/packet.h"
#include "gbn/launchBattery.h"
#include <stdlib.h>

int sendMessageGbn(int sd, struct sockaddr *dest_addr, socklen_t addrlen, void *msg, int size, void (*errorHandler)(SendError)){

    // divides message in packets
    Packet **packets;
    int numOfPackets = packetize(msg, size, &packets);

    // registers the message in the send table
    SortingEnty *sendEntry = newSortingEntry();
    sendEntry ->sd = sd;
    sendEntry ->addr = dest_addr;
    sendEntry ->addrlen = addrlen;
    sendEntry ->errorHandler = (void (*)(int)) errorHandler;
    addToSortingTable(getSendTableReference(), packets[0] ->header ->msgId, sendEntry);

    // adds the packets atomically to the battery
    if (addToLaunchBatteryAtomically(packets, numOfPackets)){

        logMsg(E, "sendMessageGbn: can't add packets to the battery\n");
        return -1;
    }

    logMsg(D, "sendMessageGbn: packets added to the battery\n");

    free(packets);

    return 0;

}

// TODO: recvMessageGbn
