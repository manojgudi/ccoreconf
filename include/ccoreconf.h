#ifndef CCORECONF_H
#define CCORECONF_H

#include "hashmap.h"
#include "sid.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct GenericValueStruct {
    enum { DICT, LIST, LEAF } type;
    // json_t of value
    json_t *jsonValue;
    void *value;

} GenericValueT;

typedef struct StackElementStruct {
    json_t *jsonValue;
    char *path;
    int parent;
    int delta;

} StackElementT;

typedef struct StackStorageStruct {
    // Array containing StackElementT objects
    StackElementT **stackElements;
    int size;
    int top;
    int capacity;

} StackStorageT;

void unwrapValues(json_t *jsonValue);

void initStackStorage(StackStorageT *stackStorage, int capacity);
bool isEmpty(StackStorageT *stackStorage);
bool isFull(StackStorageT *stackStorage);
void push(StackStorageT *stackStorage, StackElementT *stackElement);
StackElementT *pop(StackStorageT *stackStorage);
StackElementT *newStackElement(void);
void freeStackStorage(StackStorageT *stackStorage);

void lookupSID(json_t *jsonValue, SIDModelT *sidModel);
json_t *traverseCORECONF(json_t *coreconfModel, int64_t sid);
json_t *traverseCORECONFWithKeys(json_t *jsonInstance, SIDModelT *sidModel,
                                  IdentifierSIDT *sidIdentifier, int64_t keys[],
                                  size_t keyLength);

json_t *getCCORECONF(json_t *coreconfModel, SIDModelT *sidModel, int sid, int64_t keys[], size_t keyLength, int delta,
                     int depth, json_t *value);
void print_json_object(json_t *json);
#endif
