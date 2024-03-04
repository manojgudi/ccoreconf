#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/coreconfTypes.h"
#include "hashmap.h"
#include "sid.h"

typedef struct CLookup {
    int64_t childSID;
    DynamicLongListT *dynamicLongList;
} CLookupT;

typedef struct PathNode {
    int64_t parentSID;
    DynamicLongListT *sidKeys;
    struct PathNode *nextPathNode;
} PathNodeT;

typedef struct StackElementStruct {
    json_t *jsonValue;
    CoreconfValueT *coreconfValue;
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

int clookupCompare(const void *a, const void *b, void *udata);
uint64_t clookupHash(const void *item, uint64_t seed0, uint64_t seed1);
void buildCLookupHashmap(json_t *coreconfModel, struct hashmap *clookupHashmap, int64_t parentSID, int recursionDepth);
void printCLookupHashmap(struct hashmap *clookupHashmap);

void buildCLookupHashmapFromCoreconf(CoreconfValueT *coreconfValue, struct hashmap *clookupHashmap, int64_t parentSID,
                                     int recursionDepth);

// Node related function headers
PathNodeT *createPathNode(int64_t parentSID, DynamicLongListT *sidKeys);
PathNodeT *prependPathNode(PathNodeT *endNode, int64_t parentSID, DynamicLongListT *sidKeys);
void printPathNode(PathNodeT *pathNode);
void freePathNode(PathNodeT *pathNode);
PathNodeT *findRequirementForSID(uint64_t sid, struct hashmap *clookupHashmap, struct hashmap *keyMappingHashMap);
json_t *examineCoreconf(json_t *coreconfModel, DynamicLongListT *requestKeys, PathNodeT *headNode);
CoreconfValueT *examineCoreconfValue(CoreconfValueT *coreconfValue, DynamicLongListT *requestKeys, PathNodeT *headNode);

void unwrapValues(json_t *jsonValue);

void initStackStorage(StackStorageT *stackStorage, int capacity);
bool isEmpty(StackStorageT *stackStorage);
bool isFull(StackStorageT *stackStorage);
void push(StackStorageT *stackStorage, StackElementT *stackElement);
StackElementT *pop(StackStorageT *stackStorage);
StackElementT *newStackElement(void);
void freeStackStorage(StackStorageT *stackStorage);

void lookupSID(json_t *jsonValue, SIDModelT *sidModel);
CoreconfValueT *buildCoreconfModelFromJson(json_t *jsonValue, SIDModelT *sidModel, char *path, int64_t parentSID);

json_t *traverseCORECONF(json_t *coreconfModel, int64_t sid);
json_t *traverseCORECONFWithKeys(json_t *jsonInstance, SIDModelT *sidModel, IdentifierSIDT *sidIdentifier, int keys[],
                                 size_t keyLength);

json_t *getCCORECONF(json_t *coreconfModel, SIDModelT *sidModel, int sid, int keys[], size_t keyLength, int delta,
                     int depth, json_t *value);
void print_json_object(json_t *json);
