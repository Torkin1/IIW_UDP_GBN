#include "gbn/packet.h"
#include <math.h>
#include <limits.h>
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

// Max packet len shall be MTU - sizeof(headers) in order to avoid fragmentation
const int MAX_PACKET_DATA_LEN;

// Tracks id used for messages
static int LAST_MSG_ID;

Packet *newPacket(){

    Packet *packet = calloc(1, sizeof(Packet));
    packet ->header = calloc(1, sizeof(Header));

    return packet;

}

void destroyPacket(Packet *self){

    free(self ->data);
    free(self ->header);
    free(self);

};

// START SECTION /////////////////////////////////////////////////

// this function must be updated every time the struct Header changes
int calcHeaderSize(){

    return sizeof(bool) + 4 * sizeof(int);

}

int calcPacketSize(Packet *p){

    return calcHeaderSize() + (p ->header ->dataLen);

}

int calcMaxPacketDataLen(){

    return MTU - calcHeaderSize();

}

uint8_t *serializePacket(Packet *packet){

    void *serialized = calloc(calcPacketSize(packet), sizeof(uint8_t));
    uint8_t *currentByte = serialized;

    // Members of Header listed here will be serialized in order
    memcpy(currentByte, &(packet ->header ->isAck), sizeof(bool));
    currentByte += sizeof(bool);
    memcpy(currentByte, &(packet ->header ->dataLen), sizeof(int));
    currentByte += sizeof(int);
    memcpy(currentByte, &(packet ->header ->msgId), sizeof(int));
    currentByte += sizeof(int);
    memcpy(currentByte, &(packet ->header ->index), sizeof(int));
    currentByte += sizeof(int);
    memcpy(currentByte, &(packet ->header ->endIndex), sizeof(int));
    currentByte += sizeof(int);

    // serializes data
    memcpy(currentByte, packet ->data, packet ->header ->dataLen);

    return serialized;

}

Packet *deserializePacket(uint8_t *bytes){

    Packet *deserialized = newPacket();
    uint8_t *currentByte = bytes;

    // Deserializing members of Header struct
    deserialized ->header ->isAck = (bool) *currentByte;
    currentByte += sizeof(bool);
    deserialized ->header ->dataLen = (int) *currentByte;
    currentByte += sizeof(int);
    deserialized ->header ->msgId = (int) *currentByte;
    currentByte += sizeof(int);
    deserialized ->header ->index = (int) *currentByte;
    currentByte += sizeof(int);
    deserialized ->header ->endIndex = (int) *currentByte;
    currentByte += sizeof(int);

    // deserialized data is stored in a dynamically allocated buffer.
    deserialized ->data = calloc(deserialized ->header ->dataLen, sizeof(uint8_t));
    memcpy(deserialized ->data, currentByte, deserialized -> header ->dataLen);


    return deserialized;

}

// END SECTION /////////////////////////////////////////////////


// generates unique id for message
int generateMsgId(){

    int generated = LAST_MSG_ID;
    LAST_MSG_ID = (LAST_MSG_ID + 1) % INT_MAX;

    return generated;

}

// divides a message in a group of packets
int packetize(void *msg, int size, Packet ***packetsAddr){

    logMsg(D, "max data len is %d\n", calcMaxPacketDataLen());
    int numOfFullPackets = size / calcMaxPacketDataLen();
    int remain = size % calcMaxPacketDataLen();
    bool hasRemain = (remain != 0)? true : false;
    int numOfPackets = numOfFullPackets + (int) hasRemain;

    Packet **packets = calloc(numOfFullPackets + (int) hasRemain, sizeof(Packet *));

    // generates an unique id for the message
    int id = generateMsgId();

    // Prepares packets and stores them in an array
    int i, j;
    int howMany = calcMaxPacketDataLen();
    uint8_t *buf, *currentByte = msg;
    for (i = 0, j = 0; i < numOfPackets; i ++, j += howMany){

        // creates a new packet
        if (i >= numOfFullPackets)
            howMany = remain;
        packets[i] = newPacket();
        packets[i] ->header ->msgId = id;
        packets[i] ->header ->dataLen = howMany;

        // copies data in an array and stores a pointer to it in the packet
        buf = calloc(howMany, sizeof(uint8_t));
        memcpy(buf, currentByte + j, howMany);
        packets[i] ->data = buf;
    }

    *packetsAddr = packets;

    return numOfPackets;
}

/*
    IMPORTANT!
    the section below must be updated every time the Header struct changes
*/

