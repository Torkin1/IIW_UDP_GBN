#ifndef HTABLE_H_INCLUDED
#define HTABLE_H_INCLUDED

#include "dataStructures/ll.h"

#define HASHSIZE 101    // Maximum number of collision lists in hash table

// Key-value pair. Key and value buffers must be allocated dinamically
typedef struct {
    void *key;      // Key
    void *value;    // value
} Pair;

// Entries of hash table are collision lists implemented as linked lists. Value of nodes must be of Pair type
typedef Node Entry;

// Hash table implemented as an array of collision list
typedef struct {
    Entry *table[HASHSIZE];  // array of collision lists
} HashTable;

// Creates an HashTable
HashTable *newHashTable();

// Returns value paired with given key if present in table, NULL otherwise
char* getValueFromHashTable(HashTable *self, void* key, int keyLen);

// Adds given pair key-value
void addToHashTable(HashTable *self, void *key, int keyLen, void *value);

// Frees resources associated with HashTable object
void destroyHashTable(HashTable *self);

// removes given pair key-value
void removeFromHashTable(HashTable *self, void* key, int keyLen);

#endif // HTABLE_H_INCLUDED
