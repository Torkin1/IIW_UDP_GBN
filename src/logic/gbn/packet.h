#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

// max size of an IPv4 packet
#define MTU 1500

// defines the base informative unit this protocol will use to send and receive data
// Caution is needed when changing this structure because of serialization. Remember to update implementation accordingly
typedef struct header {

    bool isAck;                 // true only if this packet is an ACK
    int index;                  // index of packet in send queue
    int endIndex;               // index of last packet of this message
    int msgId;                // string id, it identifies the message where the packet has been built from
    int dataLen;                // how many bytes of data in the packet

} Header;

typedef struct packet{

    Header *header;             // packet metadata
    void *data;                 // packet data

} Packet;

Packet *newPacket();
void destroyPacket(Packet *self);

int packetize(void *msg, int size, Packet ***packetsAddr);

// return dynamically allocated byte array containing serialized packet
uint8_t *serializePacket(Packet *packet);

// deserialize data from bytes into a dynamically allocated Packet
Packet *deserializePacket(uint8_t *bytes);

int calcHeaderSize();
int calcPacketSize(Packet *p);

#endif // PACKET_H_INCLUDED
