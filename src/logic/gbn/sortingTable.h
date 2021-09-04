#ifndef SORTINGTABLE_H_INCLUDED
#define SORTINGTABLE_H_INCLUDED

#include "dataStructures/hTable.h"
#include "gbn/gbn.h"
#include "sys/socket.h"

// informations about a message are stored in a sorting entry
typedef struct sortingEntry{

    int sd;                                         // socket descriptor that will be used by sendto and recvfrom    
    struct sockaddr *addr;                          // destination or sender address
    socklen_t addrlen;                              // address length
    void (*errorHandler)(int errValue);             // function called on a new thread if the send or receive of a message failed

} SortingEntry;

// this sorting table tracks useful informations corresponding to a message
typedef struct sortingTable {

    HashTable *table;

} SortingTable;

SortingEntry *newSortingEntry();
void destroySortingEntry(SortingEntry *self);

SortingTable *newSortingTable();
void destroySortingTable(SortingTable *self);

void addToSortingTable(SortingTable *self, int msgId, SortingEntry *entry);
SortingEntry *getFromSortingTable(SortingTable *self, int msgId);
void removeFromSortingTable(SortingTable *self, int msgId);

#endif // SORTINGTABLE_H_INCLUDED
