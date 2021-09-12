#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <netinet/ip.h>

// max size of an IPv4 packet
#define MTU 25 // default: 1500. Set to calcAckSize() + 1 if you want 1 byte as max data len (currently 25)

// Packet metadata. Caution is needed when changing this structure because of serialization rules. Remember to update implementation accordingly
typedef struct header {

    bool isAck;                 // true only if this packet is an ACK
    bool isFirst;               // true if this is the first packet of its message
    int index;                  // if data: index of packet in send queue, if ACK: index of expected packet in queue
    int endIndex;               // index of last packet of this message
    int msgId;                  // string id, it identifies the message where the packet has been built from
    int dataLen;                // how many bytes of data in the packet
    int queueLen;               // needed by receiver to know when to restart to count packet indexes
    in_port_t ackPort;          // which port to send ACk

} Header;

// defines the base informative unit this protocol will use to send and receive data
typedef struct packet{

    Header *header;             // packet metadata
    void *data;                 // packet data

} Packet;

Packet *newPacket();
void destroyPacket(Packet *self);

// divides a message into packets, signed with a msgId
int packetize(void *msg, int size, Packet ***packetsAddr);

// return dynamically allocated byte array containing serialized packet
uint8_t *serializePacket(Packet *packet);

// deserialize data from bytes into a dynamically allocated Packet
Packet *deserializePacket(uint8_t *bytes);

// @return size of serialized packet header in bytes
int calcHeaderSize();

// @return size of serialized ACK in bytes
int calcAckSize();

/*
    @param p pointer to a packet
    @return size of serialized packet *p in bytes
*/
int calcPacketSize(Packet *p);

/** 
 * Max packet len shall be MTU - calckAckSize() in order to avoid fragmentation 
 * @return max number of bytes allowed to serialize data
*/
int calcMaxDataLen();

#endif // PACKET_H_INCLUDED
