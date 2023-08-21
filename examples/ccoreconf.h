#include <stdio.h>
#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>
#include <cbor.h>
#include "hashmap.h"
#include "sid.h"

typedef struct GenericValueStruct{
    enum { DICT, LIST, LEAF } type;
    // json_t of value
    json_t *jsonValue;
    void* value;

} GenericValueT;


typedef struct StackElementStruct{
    json_t *jsonValue;
    char *path;
    int parent;

} StackElementT;

typedef struct StackStorageStruct{
    // Array containing StackElementT objects
    StackElementT **stackElements;
    int size;
    int top;
    int capacity;

} StackStorageT;




void convertToCBORType(json_t *jsonItem, enum SchemaIdentifierTypeEnum identifierType, cbor_item_t *cborItem);
void convertToCORECONF(cbor_item_t *cborMap, json_t *jsonMap);
void unwrapValues(json_t *jsonValue);

void initStackStorage(StackStorageT *stackStorage, int capacity);
bool isEmpty(StackStorageT *stackStorage);
bool isFull(StackStorageT* stackStorage);
void push(StackStorageT* stackStorage, StackElementT *stackElement);
StackElementT* pop(StackStorageT *stackStorage);
StackElementT* newStackElement(void);
void freeStackStorage(StackStorageT *stackStorage);

void lookupSID(json_t *jsonValue, SIDModelT *sidModel);
