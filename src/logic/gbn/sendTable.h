#ifndef SORTINGTABLE_H_INCLUDED
#define SORTINGTABLE_H_INCLUDED

#include "dataStructures/hTable.h"
#include "gbn/gbn.h"
#include "sys/socket.h"

typedef struct sendEntry{

    void (*errorHandler)(SendError);                // function called on a new thread if the send failed
    int sd;                                         // socket descriptor that will be used by sendto
    struct sockaddr *dest_addr;                     // destination address
    socklen_t addrlen;                              // destination address length

} SendEntry;

// this sorting table tracks useful informations corresponding to a message
typedef struct sendTable {

    HashTable *table;

} SendTable;

SendEntry *newSendEntry();
void destroySendEntry(SendEntry *self);

SendTable *getSendTableReference();
void destroySendTable(SendTable *self);

void addToSendTable(SendTable *self, int msgId, SendEntry *entry);
SendEntry *getFromSendTable(SendTable *self, int msgId);
void removeFromSendTable(SendTable *self, int msgId);

#endif // SORTINGTABLE_H_INCLUDED
