#include "logger/logger.h"
#include "gbn/gbn.h"
#include "gbn/packet.h"
#include "gbn/launchBattery.h"
#include "gbn/sendTable.h"
#include <stdlib.h>

int sendMessageGbn(int sd, struct sockaddr *dest_addr, socklen_t addrlen, void *msg, int size, void (*errorHandler)(SendError)){

    // divides message in packets
    Packet **packets;
    int numOfPackets = packetize(msg, size, &packets);

    // registers the message in the send table
    SendEntry *sendEntry = newSendEntry();
    sendEntry ->sd = sd;
    sendEntry ->dest_addr = dest_addr;
    sendEntry ->addrlen = addrlen;
    sendEntry ->errorHandler = errorHandler;
    addToSendTable(getSendTableReference(), packets[0] ->header ->msgId, sendEntry);

    // TODO: set up ACK receiver

    // adds the packets atomically to the battery
    if (addToLaunchBatteryAtomically(packets, numOfPackets)){

        logMsg(E, "sendMessageGbn: can't add packets to the battery\n");
        return -1;
    }

    free(packets);

    return 0;

}