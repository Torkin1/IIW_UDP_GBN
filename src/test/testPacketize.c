#include "testSuites.h"
#include <stdlib.h>
#include <string.h>
#include "logger/logger.h"
#include "gbn/packet.h"

int testPacketize(){

    int n = 32;
    char test[n];
    memset(test, 'c', n - 1);
    test[n - 1] = '\0';
    logMsg(D, "test data prepared\n");
    Packet **packets;
    packetize(test, n, &packets);
    logMsg(D, "packetize called\n");
    Packet **buffer = calloc(4, sizeof(Packet *));
    memcpy(buffer, packets, sizeof(Packet *) * 3);

    for (int i = 0; i < 4; i ++){

        if (buffer[i] == NULL){
            logMsg(D, "FINE\n");
        }
        else{
            logMsg(D, "Packet %d\nid %d\ndata: %s\n", i, buffer[i] ->header ->msgId, buffer[i] ->data);
            destroyPacket(buffer[i]);
        }
    }

    free(packets);
    free(buffer);

    return 0;

}
