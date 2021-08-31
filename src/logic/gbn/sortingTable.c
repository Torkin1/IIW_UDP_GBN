#include "gbn/sortingTable.h"
#include "dataStructures/hTable.h"
#include "logger/logger.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


SortingTable *newSortingTable(){

    SortingTable *self = calloc(1, sizeof(SortingTable));
    self ->table = newHashTable();
    pthread_mutex_init(&(self ->lock), NULL);
    return self;

}

SortingEnty *newSortingEntry(){

    return calloc(1, sizeof(SortingEnty));

}

void destroySortingTable(SortingTable *self){

    destroyHashTable(self ->table);
    pthread_mutex_destroy(&(self ->lock));
    free(self);

}

void destroySortingEntry(SortingEnty *self){

    free(self);

}

int craftKeyFromMsgId(int msgId, int **keyAddr){

    int keySize = sizeof(int);
    *keyAddr = calloc(1, keySize);
    **keyAddr = msgId;
    return keySize;

}

void addToSortingTable(SortingTable *self, int msgId, SortingEnty *entry){

    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    addToHashTable(self ->table, key, keySize, entry);

}

SortingEnty *getFromSortingTable(SortingTable *self, int msgId){


    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    SortingEnty *entry = (SortingEnty *) getValueFromHashTable(self ->table, key, keySize);
    free(key);
    return entry;

}

void removeFromSortingTable(SortingTable *self, int msgId){

    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    removeFromHashTable(self ->table, key, keySize);
    free(key);

}



