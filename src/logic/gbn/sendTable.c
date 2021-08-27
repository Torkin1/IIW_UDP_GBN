#include "gbn/sendTable.h"
#include "dataStructures/hTable.h"
#include "logger/logger.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

SendTable *newSendTable(){

    SendTable *sendTable = calloc(1, sizeof(SendTable));
    sendTable ->table = newHashTable();
    return sendTable;

}

SendEntry *newSendEntry(){

    return calloc(1, sizeof(SendEntry));

}

void destroySendTable(SendTable *self){

    destroyHashTable(self ->table);
    free(self);

}

void destroySendEntry(SendEntry *self){

    free(self);

}

// singleton sorting table
static SendTable *sendTable;
static pthread_once_t isSendTableInitialized = PTHREAD_ONCE_INIT;

void initSendTable(){

    sendTable = newSendTable();

}

SendTable *getSendTableReference(){

    if (sendTable == NULL){

        pthread_once(&isSendTableInitialized, initSendTable);
    }

    return sendTable;

}

int craftKeyFromMsgId(int msgId, int **keyAddr){

    int keySize = sizeof(int);
    *keyAddr = calloc(1, keySize);
    **keyAddr = msgId;
    return keySize;

}

void addToSendTable(SendTable *self, int msgId, SendEntry *entry){

    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    addToHashTable(self ->table, key, keySize, entry);

}

SendEntry *getFromSendTable(SendTable *self, int msgId){


    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    SendEntry *entry = (SendEntry *) getValueFromHashTable(self ->table, key, keySize);
    free(key);
    return entry;

}

void removeFromSendTable(SendTable *self, int msgId){

    int *key;
    int keySize = craftKeyFromMsgId(msgId, &key);
    removeFromHashTable(self ->table, key, keySize);
    free(key);

}



