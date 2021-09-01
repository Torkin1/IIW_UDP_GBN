#include "gbn/catcher.h"
#include <stdbool.h>
#include <stdint.h>
#include "gbn/packet.h"
#include "logger/logger.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

static bool more = false;

int listenForData(int sd, struct sockaddr *senderAddr, socklen_t *senderAddrlen, void **msgBuf, int *size){

    uint8_t *currentByte;
    int currentByteIndex = 0;
    int expectedSeqNum = -1;    // -1 means that we haven't recevied any packet yet
    int byteBuflen = MTU;
    struct sockaddr_in sendAckAddr = {0};
    struct sockaddr_in senderAddrBuf = {0};
    int err, rcvdSeqNum, sendAckSocket, newSize;
    socklen_t senderAddrlenBuf = sizeof(struct sockaddr_in);
    uint8_t *byteBuf = calloc(byteBuflen, sizeof(uint8_t));
    Packet *packet, *ack;
    void *serializedAck;

    // initialize message buffer
    if (msgBuf == NULL){
        logMsg(E, "listenForData: must provide a non-null pointer to buffer to store msg\n");
        return -1;
    }
    *msgBuf = NULL;
    *size = 0;
    
    // listen for packets continuously
    more = true;
    while(more){

        if (recvfrom(sd, byteBuf, byteBuflen, 0, (struct sockaddr *) &senderAddrBuf, &senderAddrlenBuf) < 0){

            err = errno;
            logMsg(E, "listenForData: recvfrom failed: %s\n", strerror(err));
        }
        else{
            
            // data arrived, we check that is a data packet
            packet = deserializePacket(byteBuf);
            if (!(packet ->header ->isAck)){

                // check that packet is in order
                rcvdSeqNum = packet ->header ->index;
                if ((expectedSeqNum == -1 && packet ->header ->isFirst) || rcvdSeqNum == expectedSeqNum){
                    
                    // Check if this is the first packet.
                    if (expectedSeqNum == -1 && packet ->header ->isFirst){

                        logMsg(D, "starting reception of message %d from %s %d\n", packet ->header ->msgId, inet_ntoa(senderAddrBuf.sin_addr), ntohs(senderAddrBuf.sin_port));
                        
                        // We connect to remote sender so that we receive only its packets
                        connect(sd, (struct sockaddr *) &senderAddrBuf, senderAddrlenBuf);

                        // setup socket and target address for sending acks
                        sendAckSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);                        
                        memcpy(&sendAckAddr, &senderAddrBuf, sizeof(struct sockaddr_in));
                        sendAckAddr.sin_port = htons(packet ->header ->ackPort);

                    }

                    expectedSeqNum = (rcvdSeqNum + 1) % packet ->header ->queueLen;
                    
                    // we let sender know that we received the packet by sending an ack to the port specified in packet
                    ack = newPacket();
                    ack ->header ->isAck = true;
                    ack ->header ->index = packet ->header ->index;
                    ack ->header ->endIndex = packet ->header ->endIndex;
                    ack ->header ->msgId = packet ->header ->msgId;
                    serializedAck = serializePacket(ack);

                    if (sendto(sendAckSocket, serializedAck, calcAckSize(), MSG_NOSIGNAL, (struct sockaddr *) &sendAckAddr, sizeof(struct sockaddr_in)) < 0){
                        err = errno;
                        logMsg(E, "listenForData: failed to send ack %d to addr %s %d : %s\n", ack->header->index, inet_ntoa(sendAckAddr.sin_addr), ntohs(sendAckAddr.sin_port), strerror(err));
                    }
                    
                    else{
                        logMsg(D, "listenForData: launched ack %d to addr %s %d\n", ack->header->index, inet_ntoa(sendAckAddr.sin_addr), ntohs(sendAckAddr.sin_port));
                    }
                    free(serializedAck);
                    destroyPacket(ack);

                    // we write message of received packet in caller msg buffer
                    newSize = *size + packet ->header ->dataLen;
                    *msgBuf = reallocarray(*msgBuf, newSize, sizeof(uint8_t));
                    currentByte = *msgBuf + currentByteIndex;
                    memcpy(currentByte, packet ->data, packet ->header ->dataLen);
                    currentByteIndex += packet ->header ->dataLen;
                    *size = newSize;
                    
                    // checks if there are more packets to wait                    
                    more = !(packet ->header ->index == packet ->header ->endIndex);
                    if(!more){
                        logMsg(D, "listenForData: message %d from %s:%d has been fully received\n", packet ->header ->msgId, inet_ntoa(sendAckAddr.sin_addr), ntohs(sendAckAddr.sin_port));
                    }

                    // TODO: (re)starts timeout of last packet received. If timeout, we discard the entire message and return an error

                    destroyPacket(packet);

                }
                else {
                    logMsg(D, "discarded packet %d out of order\n", packet ->header ->index);
                }
            }
        }
    }

    
    // deliver sender data to caller
    if (senderAddr != NULL){
         memcpy(senderAddr, &senderAddrBuf, sizeof(struct sockaddr_in));
    }

    if (senderAddrlen != NULL){
        *senderAddrlen = senderAddrlenBuf;
    }
    
    // disconnects socket from sender
    struct sockaddr_in resetAddr = {0};
    resetAddr.sin_family = AF_UNSPEC;
    connect(sd, (struct sockaddr *) &resetAddr, sizeof(struct sockaddr_in));

    // closes socket for sending acks
    close(sendAckSocket);

    // cleanup
    free(byteBuf);

    return 0;
}