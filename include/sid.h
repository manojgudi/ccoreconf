#ifndef SID_H
#define SID_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

typedef struct DynamicLongListStruct {
    long *longList;
    size_t size;
} DynamicLongListT;

typedef struct KeyMappingStruct {
    int64_t key;
    DynamicLongListT *dynamicLongList;
} KeyMappingT;

DynamicLongListT *createDynamicLongList(void);
// Empties the list and frees its buffer: only for lists from createDynamicLongList
void initializeDynamicLongList(DynamicLongListT *dynamicLongList);
void addLong(DynamicLongListT *dynamicLongList, long value);
long getLong(DynamicLongListT *dynamicLongList, bool removeFromList);
long popLong(DynamicLongListT *dynamicLongList);
long peekLong(DynamicLongListT *dynamicLongList);
// Create a method to clone the Dynamiclist
void cloneDynamicLongList(DynamicLongListT *originalDynamicLongList, DynamicLongListT *clonedDynamicLongList);
// Create a method to sort the two lists
void sortDynamicLongList(DynamicLongListT *dynamicLongList, long sortedArray[]);
bool compareDynamicLongList(DynamicLongListT *dynamicLongList1, DynamicLongListT *dynamicLongList2);

void addUniqueLong(DynamicLongListT *dynamicLongList, long value);
void freeDynamicLongList(DynamicLongListT *dynamicLongList);
void printDynamicLongList(DynamicLongListT *dynamicLongList);

uint64_t keyMappingHash(const void *item, uint64_t seed0, uint64_t seed1);
int keyMappingCompare(const void *a, const void *b, void *udata);
void keyMappingFree(void *item);

void printKeyMappingT(const KeyMappingT *keyMapping);
void printKeyMappingHashMap(struct hashmap *keyMappingHashMap);

int64_t char2int64(char *keyString);
uint64_t char2uint64(char *keyString);

#endif  // SID_H
