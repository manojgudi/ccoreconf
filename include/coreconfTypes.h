#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef CORECONF_TYPES_H
#define CORECONF_TYPES_H

// Rest of header file contents go here

#define HASHMAP_TABLE_SIZE 100  // TODO Fix this to work with dynamic table size
#define CORECONF_MAX_DEPTH 20

typedef enum {
    CORECONF_ARRAY,
    CORECONF_STRING,
    CORECONF_REAL,
    CORECONF_INTEGER,
    CORECONF_TRUE,
    CORECONF_FALSE,
    CORECONF_NULL,
    CORECONF_HASHMAP
    // Future
    // CORECONF_BYTEARRAY
} coreconf_type;

typedef struct CoreconfValue {
    coreconf_type type;
    union {
        char* string_value;
        double real_value;
        uint64_t integer_value;
        struct CoreconfObject* object_value;
        struct CoreconfHashMap* map_value;
        struct CoreconfArray* array_value;
    } data;
} CoreconfValueT;

typedef struct CoreconfObject {
    uint64_t key;
    CoreconfValueT* value;
    struct CoreconfObject* next;
} CoreconfObjectT;

typedef struct CoreconfHashMap {
    CoreconfObjectT* table[HASHMAP_TABLE_SIZE];
    size_t size;
    size_t capacity;
} CoreconfHashMapT;

typedef struct CoreconfArray {
    struct CoreconfValue* elements;
    size_t size;
} CoreconfArrayT;

size_t hashKey(uint32_t v);
size_t murmurHash(uint64_t key);
void freeCoreconf(CoreconfValueT* val, bool freeValue);

CoreconfValueT* createCoreconfString(const char* value);
CoreconfValueT* createCoreconfReal(double value);
CoreconfValueT* createCoreconfBoolean(bool value);
CoreconfValueT* createCoreconfInteger(uint64_t integer);
CoreconfObjectT* createCoreconfObject();
CoreconfValueT* createCoreconfArray();
CoreconfValueT* createCoreconfHashmap();
CoreconfValueT* wrapCoreconfHashmap(CoreconfHashMapT* map);

int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value);
CoreconfValueT* getCoreconfHashMap(CoreconfHashMapT* map, uint64_t key);
void addToCoreconfArray(CoreconfValueT* arr, CoreconfValueT* value);

void printCoreconfObject(CoreconfObjectT* obj);
void printCoreconfMap(CoreconfHashMapT* map);
void printCoreconf(CoreconfValueT* val);

// Iterate over CoreconfHashMap and apply a function to each CoreconfObject value
void iterateCoreconfHashMap(CoreconfHashMapT* map, void* udata, void (*f)(CoreconfObjectT* object, void* udata));

#endif