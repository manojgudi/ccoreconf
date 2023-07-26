#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"
#include <string.h>
#include<jansson.h>
/*
 * Ideally SID file should be defined formally
 * Right now we just have strict definition for *key-mapping* in KeyMapping Struct
*/

enum IdentifierTypeEnum{
    STRING,
    UINT_8,
    UINT_16,
    RCS_ALGORITHM,
    DECIMAL64,
    BOOLEAN
};

// For printing

// Used Internally for debugging/printing
enum HashMapTypeEnum{
    KEY_MAPPING,
    IDENTIFIER_SID,
    SID_IDENTIFIER,
    IDENTIFIER_TYPE
};

typedef struct DynamicLongListStruct{
    long* longList;
    size_t size;
} DynamicLongListT;

typedef struct KeyMappingStruct {
    long key;
    DynamicLongListT* dynamicLongList;
} KeyMappingT ;

typedef struct IdentifierSIDStruct{
    char* identifier;
    long sid;
} IdentifierSIDT;

// Inverse Map
typedef struct SIDIdentifierStruct{
    long sid;
    char *identifier;
} SIDIdentifierT;

typedef struct IdentifierTypeStruct{
    char *identifier;
    enum IdentifierTypeEnum type;
} IdentifierTypeT;

typedef struct SIDModelStruct{
    // TODO Define name limit
    char* modelName;
    char* sidFilePath;

    // NOTE These need to be explicitly initialized using hashmap_new in the main

    // Contains keyMappingT Map
    struct hashmap *keyMappingHashMap;
    // Contains
    struct hashmap *identifierSIDHashMap;
    // Contains
    struct hashmap *sidIdentifierHashMap;
    // Contains
    struct hashmap *identifierTypeHashMap;

} SIDModelT;

DynamicLongListT* createDynamicLongList();
void initializeDynamicLongList(DynamicLongListT *dynamicLongList);
void addLong(DynamicLongListT* dynamicLongList, long value);
void freeDynamicLongList(DynamicLongListT *dynamicLongList);

// TODO This doesn't work
void initializeKeyMappingHashMap(struct hashmap *keyMappingHashMap);

uint64_t keyMappingHash(const void* item, uint64_t seed0, uint64_t seed1);
int keyMappingCompare(const void *a, const void *b, void *udata);

int identifierSIDCompare(const void *a, const void *b, void *udata);
uint64_t identifierSIDHash(const void* item, uint64_t seed0, uint64_t seed1);

int sidIdentifierCompare(const void *a, const void *b, void *udata);
uint64_t sidIdentifierHash(const void* item, uint64_t seed0, uint64_t seed1);

int identifierTypeCompare(const void *a, const void *b, void *udata);
uint64_t identifierTypeHash(const void* item, uint64_t seed0, uint64_t seed1);

// Tools
void buildKeyMappingHashMap(struct hashmap *keyMappingHashMap, json_t* sidFileJSON);
void printKeyMappingT(const KeyMappingT *keyMapping);
void printKeyMappingHashMap(struct hashmap *keyMappingHashMap);

void printHashMap(struct hashmap* anyHashMap, enum HashMapTypeEnum hashmapType);
void buildSIDModel(SIDModelT *sidModel, json_t *sidFileJSON);