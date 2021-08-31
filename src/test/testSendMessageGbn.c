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

// see if packets are sent correctly using wireshark on loopback interface

static char *msg = "ciao a tutti!";
static int lastIndexReceived = -1;

void testSendMessageGbn(){

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

    sendMessageGbn(sendSocket, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in), (void *) msg, strlen(msg) + 1, NULL);


    int sendAckSocket = -1;
    struct sockaddr_in *ackAddr;
    uint8_t buf[1024], *buf2;
    while(true){
        recvfrom(receiveSocket, buf, 1024, 0, NULL, NULL);
        Packet *p = deserializePacket(buf);
        if (p ->header ->index - lastIndexReceived != 1){
            continue;
        }
        lastIndexReceived = p ->header ->index;
        if (sendAckSocket < 0){
            sendAckSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            ackAddr = calloc(1, sizeof(struct sockaddr_in));
            ackAddr ->sin_family = AF_INET;
            ackAddr ->sin_port = htons(p ->header ->ackPort);
            ackAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        }
        Packet *ack = newPacket();
        ack ->header ->isAck = true;
        ack ->header ->index = p ->header ->index;
        ack ->header ->endIndex = p ->header ->endIndex;
        ack ->header ->msgId = p ->header ->msgId;
        buf2 = serializePacket(ack);
        sendto(sendAckSocket, buf2, calcAckSize(), MSG_NOSIGNAL, (struct sockaddr *) ackAddr, sizeof(struct sockaddr_in));
        logMsg(D, "testSendMessageGbn: launched ack %d to port %d\n", ack ->header ->index, p ->header ->ackPort);
    }
}
