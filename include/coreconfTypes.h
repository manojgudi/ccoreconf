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
#define CORECONF_MAX_LOOP 50
typedef enum {
    CORECONF_ARRAY,
    CORECONF_STRING,
    CORECONF_REAL,

    CORECONF_INT_8,
    CORECONF_INT_16,
    CORECONF_INT_32,
    CORECONF_INT_64,

    CORECONF_UINT_8,
    CORECONF_UINT_16,
    CORECONF_UINT_32,
    CORECONF_UINT_64,

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

CoreconfValueT* createCoreconfInt8(int8_t value);
CoreconfValueT* createCoreconfInt16(int16_t value);
CoreconfValueT* createCoreconfInt32(int32_t value);
CoreconfValueT* createCoreconfInt64(int64_t value);

CoreconfValueT* createCoreconfUint8(uint8_t value);
CoreconfValueT* createCoreconfUint16(uint16_t value);
CoreconfValueT* createCoreconfUint32(uint32_t value);
CoreconfValueT* createCoreconfUint64(uint64_t value);

CoreconfObjectT* createCoreconfObject(void);
CoreconfValueT* createCoreconfArray(void);
CoreconfValueT* createCoreconfHashmap(void);
CoreconfValueT* wrapCoreconfHashmap(CoreconfHashMapT* map);

int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value);
CoreconfValueT* getCoreconfHashMap(CoreconfHashMapT* map, uint64_t key);
void addToCoreconfArray(CoreconfValueT* arr, CoreconfValueT* value);

void printCoreconfObject(CoreconfObjectT* obj);
void printCoreconfMap(CoreconfHashMapT* map);
void printCoreconf(CoreconfValueT* val);

// Iterate over CoreconfHashMap and apply a function to each CoreconfObject value
void iterateCoreconfHashMap(CoreconfHashMapT* map, void* udata, void (*f)(CoreconfObjectT* object, void* udata));

// Allow us to use non-standard function
extern char* strdup(const char*);
#endif