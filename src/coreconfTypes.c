#include "../include/coreconfTypes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/sid.h"

CoreconfValueT* createCoreconfString(const char* value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_STRING;
    val->data.string_value = (char*)strdup(value);
    return val;
}

// Copy `length` bytes of a (not necessarily NUL-terminated) text string
CoreconfValueT* createCoreconfStringLength(const char* value, size_t length) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    if (val == NULL) return NULL;
    val->type = CORECONF_STRING;
    val->data.string_value = malloc(length + 1);
    if (val->data.string_value == NULL) {
        free(val);
        return NULL;
    }
    if (length > 0) memcpy(val->data.string_value, value, length);
    val->data.string_value[length] = '\0';
    return val;
}

CoreconfValueT* createCoreconfBytes(const uint8_t* data, size_t length) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    if (val == NULL) return NULL;
    val->type = CORECONF_BYTES;
    val->data.bytes_value.length = length;
    val->data.bytes_value.data = NULL;
    if (length > 0) {
        val->data.bytes_value.data = malloc(length);
        if (val->data.bytes_value.data == NULL) {
            free(val);
            return NULL;
        }
        memcpy(val->data.bytes_value.data, data, length);
    }
    return val;
}

CoreconfValueT* createCoreconfTag(uint64_t number, CoreconfValueT* value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    if (val == NULL) return NULL;
    val->type = CORECONF_TAG;
    val->data.tag_value.number = number;
    val->data.tag_value.value = value;
    return val;
}

CoreconfValueT* createCoreconfNull(void) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    if (val == NULL) return NULL;
    val->type = CORECONF_NULL;
    return val;
}

bool isCoreconfTag(const CoreconfValueT* val, uint64_t number) {
    return val != NULL && val->type == CORECONF_TAG && val->data.tag_value.number == number;
}

CoreconfValueT* getCoreconfTagValue(CoreconfValueT* val) {
    while (val != NULL && val->type == CORECONF_TAG) {
        val = val->data.tag_value.value;
    }
    return val;
}

CoreconfValueT* createCoreconfReal(double value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_REAL;
    val->data.real_value = value;
    return val;
}

CoreconfValueT* createCoreconfInt8(int8_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_8;
    val->data.i8 = value;
    return val;
}

CoreconfValueT* createCoreconfInt16(int16_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_16;
    val->data.i16 = value;
    return val;
}

CoreconfValueT* createCoreconfInt32(int32_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_32;
    val->data.i32 = value;
    return val;
}

CoreconfValueT* createCoreconfInt64(int64_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_64;
    val->data.i64 = value;
    return val;
}

CoreconfValueT* createCoreconfUint8(uint8_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_8;
    val->data.u8 = value;
    return val;
}

CoreconfValueT* createCoreconfUint16(uint16_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_16;
    val->data.u16 = value;
    return val;
}

CoreconfValueT* createCoreconfUint32(uint32_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_32;
    val->data.u32 = value;
    return val;
}

CoreconfValueT* createCoreconfUint64(uint64_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_64;
    val->data.u64 = value;
    return val;
}

CoreconfObjectT* createCoreconfObject(void) {
    CoreconfObjectT* obj = malloc(sizeof(CoreconfObjectT));
    obj->key = 0;
    obj->value = NULL;
    obj->next = NULL;
    return obj;
}

CoreconfValueT* createCoreconfBoolean(bool value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = value ? CORECONF_TRUE : CORECONF_FALSE;
    return val;
}

CoreconfValueT* createCoreconfHashmap(void) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_HASHMAP;
    val->data.map_value = malloc(sizeof(CoreconfHashMapT));
    val->data.map_value->size = 0;
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        val->data.map_value->table[i] = NULL;
    }
    return val;
}

// Insert Coreconf Object into CoreconfHashMap
int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value) {
    int loopCount = 0;
    size_t index = hashKey(key);

    CoreconfObjectT* coreconfObject_ = createCoreconfObject();
    // Check if malloc failed
    if (coreconfObject_ == NULL) {
        return -1;
    }
    coreconfObject_->key = key;
    coreconfObject_->value = value;

    if (map->table[index] != NULL) {
        // Delete the object if it already exists and overwrite it
        CoreconfObjectT* current = map->table[index];

        // Overwrite if the key already exists
        while (current != NULL) {
            if (loopCount > CORECONF_MAX_LOOP) {
                printf("Loop count exceeded CORECONF_MAX_LOOP\n");
                return -1;
            }
            if (current->key == key) {
                // If both existing and new values are hashmaps, recursively merge
                if (current->value->type == CORECONF_HASHMAP && value->type == CORECONF_HASHMAP) {
                    // Iterate through all entries in the update hashmap
                    for (size_t j = 0; j < HASHMAP_TABLE_SIZE; j++) {
                        CoreconfObjectT* updateEntry = value->data.map_value->table[j];
                        while (updateEntry != NULL) {
                            int result = insertCoreconfHashMap(current->value->data.map_value, updateEntry->key,
                                                               updateEntry->value);
                            if (result != 0) {
                                free(coreconfObject_);
                                return result;
                            }
                            updateEntry = updateEntry->next;
                        }
                    }
                    free(coreconfObject_);
                    return 0;
                }
                // Otherwise, replace the value
                freeCoreconf(current->value, true);
                current->value = value;
                free(coreconfObject_);
                return 0;
            }
            current = current->next;
            loopCount++;
        }
        coreconfObject_->next = map->table[index];
        map->table[index] = coreconfObject_;
    } else {
        map->table[index] = coreconfObject_;
    }
    map->size++;
    return 0;
}

// Get Value from CoreconfHashMap
CoreconfValueT* getCoreconfHashMap(CoreconfHashMapT* map, uint64_t key) {
    size_t index = hashKey(key);
    CoreconfObjectT* current = map->table[index];
    while (current != NULL) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

void freeCoreconfHashMap(CoreconfHashMapT* map) {
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        CoreconfObjectT* current = map->table[i];
        while (current != NULL) {
            CoreconfObjectT* next = current->next;
            // Map values are allocated one by one (unlike array elements), so free each one entirely
            freeCoreconf(current->value, true);
            free(current);
            current = next;
        }
    }
    free(map);
}

void printCoreconfMap(CoreconfHashMapT* map) {
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        CoreconfObjectT* current = map->table[i];
        if (current != NULL) {
            while (current != NULL) {
                printCoreconfObject(current);
                current = current->next;
            }
        }
    }
}

void printCoreconfObject(CoreconfObjectT* obj) {
    if (!obj) return;
    printf("Key: %" PRIu64 " Value: ", obj->key);
    printCoreconf(obj->value);
    printf(", ");
    // printf("\n");
}

bool isTypeUint(uint64_t type) {
    return (type == CORECONF_UINT_8 || type == CORECONF_UINT_16 || type == CORECONF_UINT_32 ||
            type == CORECONF_UINT_64);
}

bool isTypeInt(uint64_t type) {
    return (type == CORECONF_INT_8 || type == CORECONF_INT_16 || type == CORECONF_INT_32 || type == CORECONF_INT_64);
}

// Method used in examineCoreconf to match the SIDKey value,
// and to keep all integers stored in 64 bit values, since CBOR does not have a distinction
uint64_t getCoreconfValueAsUint64(CoreconfValueT* val) {
    // e.g. an identityref inside a union (tag 45) reads as its SID
    val = getCoreconfTagValue(val);
    // NULL check to prevent dereference when getCoreconfHashMap returns NULL
    if (val == NULL) {
        return 0;
    }
    switch (val->type) {
        case CORECONF_INT_64:
            return val->data.i64;
        case CORECONF_REAL:
            if (val->data.real_value >= 0) {
                return (uint64_t)val->data.real_value;
            } else {
                return 0;
            }
        case CORECONF_INT_16:
            return (uint64_t)val->data.i16;
        case CORECONF_INT_32:
            return (uint64_t)val->data.i32;
        case CORECONF_INT_8:
            return (uint64_t)val->data.i8;
        case CORECONF_UINT_64:
            return val->data.u64;
        case CORECONF_UINT_16:
            return val->data.u16;
        case CORECONF_UINT_32:
            return val->data.u32;
        case CORECONF_UINT_8:
            return val->data.u8;
        case CORECONF_STRING:
            return char2uint64(val->data.string_value);
        default:
            return 0;
    }
}

// Method used to keep all integers stored in 64 bit values, since CBOR does not have a distinction
uint64_t getCoreconfValueAsInt64(CoreconfValueT* val) {
    val = getCoreconfTagValue(val);
    // NULL check to prevent dereference when getCoreconfHashMap returns NULL
    if (val == NULL) {
        return 0;
    }
    switch (val->type) {
        case CORECONF_INT_64:
            return val->data.i64;
        case CORECONF_REAL:
            if (val->data.real_value >= 0) {
                return (int64_t)val->data.real_value;
            } else {
                return 0;
            }
        case CORECONF_INT_16:
            return val->data.i16;
        case CORECONF_INT_32:
            return val->data.i32;
        case CORECONF_INT_8:
            return val->data.i8;
        case CORECONF_UINT_64:
            return (int64_t)val->data.u64;
        case CORECONF_UINT_16:
            return (int64_t)val->data.u16;
        case CORECONF_UINT_32:
            return (int64_t)val->data.u32;
        case CORECONF_UINT_8:
            return (int64_t)val->data.u8;
        case CORECONF_STRING:
            return char2int64(val->data.string_value);
        default:
            return 0;
    }
}

void printCoreconf(CoreconfValueT* val) {
    if (!val) return;
    switch (val->type) {
        case CORECONF_STRING:
            printf("%s", val->data.string_value);
            break;
        case CORECONF_REAL:
            printf("%f", val->data.real_value);
            break;
        case CORECONF_INT_8:
            printf("%d", (int)val->data.i8);
            break;
        case CORECONF_INT_16:
            printf("%d", (int)val->data.i16);
            break;
        case CORECONF_INT_32:
            printf("%d", (int)val->data.i32);
            break;
        case CORECONF_INT_64:
            printf("%" PRId64, val->data.i64);
            break;
        case CORECONF_UINT_8:
            printf("%u", (uint8_t)val->data.u8);
            break;
        case CORECONF_UINT_16:
            printf("%u", (uint16_t)val->data.u16);
            break;
        case CORECONF_UINT_32:
            printf("%u", (uint32_t)val->data.u32);
            break;
        case CORECONF_UINT_64:
            printf("%" PRIu64, val->data.u64);
            break;
        case CORECONF_TRUE:
            printf("true");
            break;
        case CORECONF_FALSE:
            printf("false");
            break;
        case CORECONF_NULL:
            printf("null");
            break;
        case CORECONF_ARRAY:
            printf("[");
            for (size_t i = 0; i < val->data.array_value->size; i++) {
                printCoreconf(&val->data.array_value->elements[i]);
                if (i != val->data.array_value->size - 1) printf(", ");
            }
            printf("]");
            break;
        case CORECONF_HASHMAP:
            printCoreconfMap(val->data.map_value);
            break;
        case CORECONF_BYTES:
            // CBOR diagnostic notation: h'0a1b'
            printf("h'");
            for (size_t i = 0; i < val->data.bytes_value.length; i++) {
                printf("%02x", val->data.bytes_value.data[i]);
            }
            printf("'");
            break;
        case CORECONF_TAG:
            printf("%" PRIu64 "(", val->data.tag_value.number);
            printCoreconf(val->data.tag_value.value);
            printf(")");
            break;
    }
}

CoreconfValueT* createCoreconfArray(void) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_ARRAY;
    val->data.array_value = malloc(sizeof(CoreconfArrayT));
    val->data.array_value->elements = NULL;
    val->data.array_value->size = 0;
    return val;
}

// Grows by exactly one element: no spare room, since RAM is scarce on embedded
// targets.  Decoded arrays are allocated once at their full length instead
// (see _parse_array), so this is only used for appends at runtime.
void addToCoreconfArray(CoreconfValueT* arr, CoreconfValueT* value) {
    CoreconfArrayT* array = arr->data.array_value;
    CoreconfValueT* elements = realloc(array->elements, (array->size + 1) * sizeof(CoreconfValueT));
    if (elements == NULL) {
        return;  // Out of memory: the array is left unchanged
    }
    array->elements = elements;
    array->elements[array->size] = *value;
    array->size++;
}

// Search array for element with matching key, update if found, append if not
// keySID and parentSID are used to calculate delta SID
// Returns 0 on success, -1 on error
int updateCoreconfArrayByKey(CoreconfValueT* arr, uint64_t keySID, uint64_t parentSID, uint64_t keyValue,
                             CoreconfValueT* newValue) {
    if (arr == NULL || arr->type != CORECONF_ARRAY || newValue == NULL) {
        return -1;
    }

    // Calculate delta SID for key lookup
    uint64_t deltaSID = keySID - parentSID;

    // Search through array elements for one with matching key
    for (size_t i = 0; i < arr->data.array_value->size; i++) {
        CoreconfValueT* element = &arr->data.array_value->elements[i];

        // Element must be a hashmap to contain keys
        if (element->type != CORECONF_HASHMAP) {
            continue;
        }

        // Check if this element has the key we're looking for (using delta SID)
        CoreconfValueT* keyInElement = getCoreconfHashMap(element->data.map_value, deltaSID);
        if (keyInElement != NULL) {
            uint64_t elementKeyValue = getCoreconfValueAsUint64(keyInElement);
            if (elementKeyValue == keyValue) {
                // Found it - update this element
                freeCoreconf(element, false);
                *element = *newValue;
                return 0;
            }
        }
    }

    // Key not found - append new element
    addToCoreconfArray(arr, newValue);
    return 0;
}

void freeCoreconf(CoreconfValueT* val, bool freeValue) {
    if (!val) return;
    if (val->type == CORECONF_STRING)
        free(val->data.string_value);
    else if (val->type == CORECONF_ARRAY) {
        for (size_t i = 0; i < val->data.array_value->size; i++) {
            freeCoreconf(&val->data.array_value->elements[i], false);
        }
        free(val->data.array_value->elements);
        free(val->data.array_value);
    } else if (val->type == CORECONF_HASHMAP) {
        freeCoreconfHashMap(val->data.map_value);
    } else if (val->type == CORECONF_BYTES) {
        free(val->data.bytes_value.data);
    } else if (val->type == CORECONF_TAG) {
        freeCoreconf(val->data.tag_value.value, true);
    }

    // freeValue is true when the value is not part of an array
    if (freeValue) free(val);
}

// 64-bit MurmurHash3-inspired hash for CoreconfHashMap Keys
static size_t murmurHash(uint64_t key);

size_t hashKey(uint64_t key) { return murmurHash(key); }

size_t murmurHash(uint64_t key) {
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = 0 ^ (8 * m);
    uint64_t k = key;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h % HASHMAP_TABLE_SIZE;
}
