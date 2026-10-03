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
    CORECONF_HASHMAP,
    CORECONF_BYTES,  // CBOR byte string (YANG binary)
    CORECONF_TAG     // CBOR tag around another value (RFC 9254 unions: 43 bits, 44 enum, 45 identityref, 46 instance-id)
} coreconf_type;

typedef struct CoreconfValue {
    coreconf_type type;
    union {
        char* string_value;
        double real_value;
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;
        int32_t i32;
        uint32_t u32;
        int64_t i64;
        uint64_t u64;
        struct CoreconfObject* object_value;
        struct CoreconfHashMap* map_value;
        struct CoreconfArray* array_value;
        struct {
            uint8_t* data;
            size_t length;
        } bytes_value;
        struct {
            uint64_t number;
            struct CoreconfValue* value;  // Owned: freed with the tag
        } tag_value;
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
} CoreconfHashMapT;

typedef struct CoreconfArray {
    struct CoreconfValue* elements;
    size_t size;
} CoreconfArrayT;

size_t hashKey(uint64_t key);
void freeCoreconf(CoreconfValueT* val, bool freeValue);

CoreconfValueT* createCoreconfString(const char* value);
CoreconfValueT* createCoreconfStringLength(const char* value, size_t length);
CoreconfValueT* createCoreconfBytes(const uint8_t* data, size_t length);
// Takes ownership of `value`
CoreconfValueT* createCoreconfTag(uint64_t number, CoreconfValueT* value);
CoreconfValueT* createCoreconfNull(void);
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

int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value);
CoreconfValueT* getCoreconfHashMap(CoreconfHashMapT* map, uint64_t key);
void addToCoreconfArray(CoreconfValueT* arr, CoreconfValueT* value);
void freeCoreconfHashMap(CoreconfHashMapT* map);
int updateCoreconfArrayByKey(CoreconfValueT* arr, uint64_t keySID, uint64_t parentSID, uint64_t keyValue,
                             CoreconfValueT* newValue);

void printCoreconfObject(CoreconfObjectT* obj);
void printCoreconfMap(CoreconfHashMapT* map);
void printCoreconf(CoreconfValueT* val);

bool isTypeUint(uint64_t type);
bool isTypeInt(uint64_t type);

// True if `val` is a CORECONF_TAG with tag `number`
bool isCoreconfTag(const CoreconfValueT* val, uint64_t number);
// The value inside a tag (through nested tags); `val` itself if it is not a tag
CoreconfValueT* getCoreconfTagValue(CoreconfValueT* val);

// get uint64_t  from any CoreconfValueT of REAL or UINT or INT type (looks through tags)
uint64_t getCoreconfValueAsUint64(CoreconfValueT* val);
uint64_t getCoreconfValueAsInt64(CoreconfValueT* val);

// Allow us to use non-standard function
extern char* strdup(const char*);
#endif
