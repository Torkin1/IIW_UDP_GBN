#include "gbn/gbn.h"
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "gbn/launcher.h"
#include "gbn/packet.h"
#include "logger/logger.h"
#include <stdio.h>
#include <stdbool.h>

// tested with MTU = 20 and winsize = 10
// see if packets are sent correctly using wireshark on loopback interface

char *msg = "ciao a tutti!";

// TODO: test case when acks arrive out of order
void testSendMessageGbn(){

    int sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in *sendAddr = calloc(1, sizeof(struct sockaddr_in));
    sendAddr ->sin_family = AF_INET;
    sendAddr ->sin_port = 32772;
    sendAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int sendAckSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in *ackAddr = calloc(1, sizeof(struct sockaddr_in));
    ackAddr ->sin_family = AF_INET;
    ackAddr ->sin_port = ACK_LISTENING_PORT;
    ackAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    bool fakeSent = false;
    for (int j = 0; j < 2; j ++){
        sendMessageGbn(sendSocket, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in), (void *) msg, strlen(msg) + 1, NULL);

        logMsg(D, "about to send one ack per sec in ");
        for (int i = 5; i > 0; i --)
        {
            printf("%d ", i);
            fflush(0);
            sleep(1);
        }
        printf("\n");
        Packet *ack;
        uint8_t *buf;
        for (int i = 0; i < 14 ; i ++)
        {

            ack = newPacket();
            ack ->header ->dataLen = 0;
            ack ->header ->endIndex = strlen(msg);
            ack ->header ->isAck = true;
            if (i == 2 && !fakeSent){
                i = 3;
            }
            ack ->header ->index = i;
            if (i == 3 && !fakeSent){
                i = 1;
                fakeSent = true;
            }
            ack ->header ->msgId = j;
            buf = serializePacket(ack);
            if (i != 5){
                sendto(sendAckSocket, buf, calcAckSize(), MSG_NOSIGNAL, (struct sockaddr *) ackAddr, sizeof(struct sockaddr_in));
                logMsg(D, "launched ack %d\n", ack ->header ->index);
            }
            sleep(1);
            free(buf);
            destroyPacket(ack);
        }
    }
    pause();




}
