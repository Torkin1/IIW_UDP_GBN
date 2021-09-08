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

// @return dinamically allocated SortingEntry object
SortingEntry *newSortingEntry();

/*
    @param self pointer to SortingEntry to destroy
*/
void destroySortingEntry(SortingEntry *self);

// @return a new SortingTable object
SortingTable *newSortingTable();

// @param self *self will be destroyed along with all it's entries. Keys and values of entries will be destroyed too
void destroySortingTable(SortingTable *self);

/*
@param self *SortingTable which will hold the new entry
@param msgId key that can be used later to retrieve the entry
@param entry pointer to dinamically allocated SortingEntry to add
*/
void addToSortingTable(SortingTable *self, int msgId, SortingEntry *entry);

/*
    @param self pointer to SortingTable object
    @param msgId key used to retrieve the desired entry
    @return the entry corresponding to the key
*/
SortingEntry *getFromSortingTable(SortingTable *self, int msgId);

/*
    Destroys the entry corresponding to msgId and removes it from the table
    @param self pointer to SortingTable object
    @param msgId key used to retrieve the desired entry

*/
void removeFromSortingTable(SortingTable *self, int msgId);

#endif // SORTINGTABLE_H_INCLUDED
