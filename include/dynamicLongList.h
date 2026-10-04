#ifndef DYNAMIC_LONG_LIST_H
#define DYNAMIC_LONG_LIST_H

#include <stdbool.h>
#include <stddef.h>

// A growable list of longs (SIDs, list key values), used as a stack by popLong
typedef struct DynamicLongListStruct {
    long *longList;
    size_t size;
} DynamicLongListT;

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

#endif  // DYNAMIC_LONG_LIST_H
