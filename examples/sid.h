#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"
#include <string.h>
/*
 * Ideally SID file should be defined formally
 * Right now we just have strict definition for *key-mapping* in KeyMapping Struct
*/


typedef struct DynamicLongListStruct{
    long* longList;
    size_t size;
} DynamicLongListT;

typedef struct KeyMappingStruct {
    long key;
    DynamicLongListT* dynamicLongList;
} KeyMappingT ;

DynamicLongListT* createDynamicLongList();
void initializeDynamicLongList(DynamicLongListT *dynamicLongList);
void addLong(DynamicLongListT* dynamicLongList, long value);
void freeDynamicLongList(DynamicLongListT *dynamicLongList);

// 
void initializeKeyMappingHashMap(struct hashmap *keyMappingHashMap);
