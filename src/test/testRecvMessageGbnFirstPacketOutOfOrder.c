#include "gbn/gbn.h"
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "gbn/launcher.h"
#include "gbn/launchBattery.h"
#include "gbn/packet.h"
#include "logger/logger.h"
#include <stdio.h>
#include <stdbool.h>
#include "testSuites.h"
#include <errno.h>

// see if packets are sent correctly using wireshark on loopback interface

static void *rcvd;
static int rcvdSize;

void testRecvMessageGbnFirstPacketOutOfOrder(){

    
    logMsg(D, "test started\n");
    
    int sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in *sendAddr = calloc(1, sizeof(struct sockaddr_in));
    sendAddr ->sin_family = AF_INET;
    sendAddr ->sin_port = htons(32772);
    sendAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int receiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in *recvAddr = calloc(1, sizeof(struct sockaddr_in));
    recvAddr ->sin_family = AF_INET;
    recvAddr ->sin_port = htons(32772);
    recvAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(receiveSocket, (struct sockaddr*) recvAddr, sizeof(struct sockaddr_in));

    Packet *p;
    int times = 2;
    void *buf;
    bool sendFirst = true;
    for (int i = 0; i < times; i ++){
        logMsg(D, "iteration %d\n", i);
        p = newPacket();
        p ->header ->dataLen = 0;
        p ->header ->index = i;
        p ->header ->endIndex = times;
        if (i == 0){
            p ->header ->isFirst = false;
        }
        buf = serializePacket(p);
        if (i != 0 || (i == 0 && sendFirst)){
            if (sendto(sendSocket, buf, calcPacketSize(p), 0, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in)) < 0){
                int err = errno;
                logMsg(D, "sendto failed %s\n", strerror(err));
            }

            logMsg(D, "launched packet %d\n", p ->header ->index);
            
        }
    }
    
    recvMessageGbn(receiveSocket, NULL, NULL, &rcvd, &rcvdSize);


    free(sendAddr);
    free(recvAddr);


}
