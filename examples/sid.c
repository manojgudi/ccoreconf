#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"
#include "sid.h"
#include <string.h>
#include <math.h>

#define SID_LENGTH 20

DynamicLongListT *createDynamicLongList(){
    DynamicLongListT *dynamicLongList  = (DynamicLongListT *) malloc(sizeof(DynamicLongListT));
    
    if (dynamicLongList == NULL){
        fprintf(stderr, "Failed malloc'ing Dynamic long list");
        return NULL;
    } 
    dynamicLongList->longList = NULL;
    dynamicLongList->size = 0;
    return dynamicLongList;
}

void initializeDynamicLongList(DynamicLongListT *dynamicLongList){
    // If its NULL, do nothing;
    if (!dynamicLongList)
        return;
    dynamicLongList->longList = malloc(sizeof(long));
}

void addLong(DynamicLongListT *dynamicLongList, long value){
    size_t currentListSize;
    if (dynamicLongList == NULL){
        currentListSize = 0;
    }else{
        currentListSize = dynamicLongList->size;
    }

    dynamicLongList->longList = (long*) realloc(dynamicLongList->longList, (currentListSize+1) * sizeof(long));
    // Check if realloc happened properly, if no, then realloc failed and longList will be NULL
    if (!dynamicLongList->longList){
        fprintf(stderr, "Failed realloc'ing long list");
        return;
    }
    dynamicLongList->size = currentListSize+1;
    dynamicLongList->longList[currentListSize] = value;
}

void freeDynamicLongList(DynamicLongListT *dynamicLongList){
    // If its NULL, then do nothing
    if (!dynamicLongList)
        return;
    free(dynamicLongList->longList);
    free(dynamicLongList);
}

int keyMappingCompare(const void *a, const void *b, void *udata){
    const KeyMappingT *keyMapping1 = (KeyMappingT*) a;
    const KeyMappingT *keyMapping2 = (KeyMappingT*) b;
    // return strcmp(keyMapping1->key, keyMapping2->key);
    return (keyMapping1->key != keyMapping2->key);
}

uint64_t keyMappingHash(const void* item, uint64_t seed0, uint64_t seed1){
    const KeyMappingT *keyMapping = (KeyMappingT *) item;
    //char keyString[SID_LENGTH];
    //sprintf(keyString, "%lu", keyMapping->key);
    return hashmap_sip(&keyMapping->key, log(keyMapping->key),seed0, seed1);
}

void initializeKeyMappingHashMap(struct hashmap *keyMappingHashMap){
    keyMappingHashMap = hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
}
