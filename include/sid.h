#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
/*
 * Ideally SID file should be defined formally
 * Right now we just have strict definition for *key-mapping* in KeyMapping Struct
 */

// Don't forget to update the SchemaIdentifierTypeStrings
enum SchemaIdentifierTypeEnum {
    STRING,
    UINT_8,
    UINT_16,
    UINT_32,
    UINT_64,
    RCS_ALGORITHM,
    DECIMAL64,
    BOOLEAN,
    IDENTITY_REF
};

// For printing

// Used Internally for debugging/printing
enum HashMapTypeEnum { KEY_MAPPING, IDENTIFIER_SID, SID_IDENTIFIER, IDENTIFIER_TYPE, CLOOKUP_TYPE };

typedef struct DynamicLongListStruct {
    long *longList;
    size_t size;
} DynamicLongListT;

typedef struct KeyMappingStruct {
    int64_t key;
    DynamicLongListT *dynamicLongList;
} KeyMappingT;

typedef struct IdentifierSIDStruct {
    char *identifier;
    long sid;
} IdentifierSIDT;

// Inverse Map
typedef struct SIDIdentifierStruct {
    long sid;
    char *identifier;
} SIDIdentifierT;

typedef struct IdentifierTypeStruct {
    char *identifier;
    enum SchemaIdentifierTypeEnum type;
} IdentifierTypeT;

typedef struct SIDModelStruct {
    // TODO Define name limit
    char *modelName;
    char *sidFilePath;

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

DynamicLongListT *createDynamicLongList(void);
void initializeDynamicLongList(DynamicLongListT *dynamicLongList);
void addLong(DynamicLongListT *dynamicLongList, long value);
long popLong(DynamicLongListT *dynamicLongList);
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

int identifierSIDCompare(const void *a, const void *b, void *udata);
uint64_t identifierSIDHash(const void *item, uint64_t seed0, uint64_t seed1);

int sidIdentifierCompare(const void *a, const void *b, void *udata);
uint64_t sidIdentifierHash(const void *item, uint64_t seed0, uint64_t seed1);

int identifierTypeCompare(const void *a, const void *b, void *udata);
uint64_t identifierTypeHash(const void *item, uint64_t seed0, uint64_t seed1);

void printKeyMappingT(const KeyMappingT *keyMapping);
void printKeyMappingHashMap(struct hashmap *keyMappingHashMap);

void printHashMap(struct hashmap *anyHashMap, enum HashMapTypeEnum hashmapType);

// Path format function to remove trailing '\'
void removeTrailingSlashFromPath(const char *qualifiedPath, char *formattedPath);
int64_t char2int64(char *keyString);
char *int2str(char *keyString, int64_t keyInt64);
char *getSubstringAfterLastColon(const char *input);
