#include "../include/coreconfTypes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CoreconfValueT* createCoreconfString(const char* value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_STRING;
    val->data.string_value = (char*)strdup(value);
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
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfInt16(int16_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_16;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfInt32(int32_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_32;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfInt64(int64_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INT_64;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfUint8(uint8_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_8;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfUint16(uint16_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_16;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfUint32(uint32_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_32;
    val->data.integer_value = value;
    return val;
}

CoreconfValueT* createCoreconfUint64(uint64_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_UINT_64;
    val->data.integer_value = value;
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
    val->data.map_value->capacity = HASHMAP_TABLE_SIZE;
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        val->data.map_value->table[i] = NULL;
    }
    return val;
}

// Create CoreconfValueT* from an existing CoreconfHashMapT
CoreconfValueT* wrapCoreconfHashmap(CoreconfHashMapT* map) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_HASHMAP;
    val->data.map_value = map;
    return val;
}

// Insert Coreconf Object into CoreconfHashMap
int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value) {
    int loopCount = 0;
    size_t index = hashKey((uint32_t)key);

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
    size_t index = hashKey((uint32_t)key);
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
            if (current->next != NULL) {
                CoreconfObjectT* next = current->next;
                // Free the value only if its not null
                if (current->value != NULL) freeCoreconf(current->value, false);
                free(current);
                current = next;
            } else {
                free(current);
                current = NULL;
            }
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
    printf("Key: %d Value: ", (int)obj->key);
    printCoreconf(obj->value);
    printf(", ");
    // printf("\n");
}

// Method used in examineCoreconf to match the sidKey value
uint64_t getCoreconfValueAsUint64(CoreconfValueT* val) {
    if (val->type == CORECONF_INT_64)
        return val->data.integer_value;
    else if (val->type == CORECONF_REAL || val->data.real_value >= 0)
        return (uint64_t)val->data.real_value;
    else if (val->type == CORECONF_INT_16)
        return (uint64_t)val->data.integer_value;
    else if (val->type == CORECONF_INT_32)
        return (uint64_t)val->data.integer_value;
    else if (val->type == CORECONF_INT_8)
        return (uint64_t)val->data.integer_value;
    else if (val->type == CORECONF_UINT_64)
        return val->data.integer_value;
    else if (val->type == CORECONF_UINT_16)
        return val->data.integer_value;
    else if (val->type == CORECONF_UINT_32)
        return val->data.integer_value;
    else if (val->type == CORECONF_UINT_8)
        return val->data.integer_value;
    else
        return 0;
}

void printCoreconf(CoreconfValueT* val) {
    if (!val) return;
    if (val->type == CORECONF_STRING)
        printf("%s", val->data.string_value);
    else if (val->type == CORECONF_REAL)
        printf("%f", val->data.real_value);
    else if (val->type == CORECONF_INT_8)
        printf("%d", (int)val->data.integer_value);
    else if (val->type == CORECONF_INT_16)
        printf("%d", (int)val->data.integer_value);
    else if (val->type == CORECONF_INT_32)
        printf("%d", (int)val->data.integer_value);
    else if (val->type == CORECONF_INT_64)
        printf("%" PRId64, val->data.integer_value);
    else if (val->type == CORECONF_UINT_8)
        printf("%u", (uint8_t)val->data.integer_value);
    else if (val->type == CORECONF_UINT_16)
        printf("%u", (uint16_t)val->data.integer_value);
    else if (val->type == CORECONF_UINT_32)
        printf("%u", (uint32_t)val->data.integer_value);
    else if (val->type == CORECONF_UINT_64)
        printf("%" PRIu64, val->data.integer_value);

    else if (val->type == CORECONF_TRUE)
        printf("true");
    else if (val->type == CORECONF_FALSE)
        printf("false");
    else if (val->type == CORECONF_NULL)
        printf("null");
    else if (val->type == CORECONF_ARRAY) {
        printf("[");
        for (size_t i = 0; i < val->data.array_value->size; i++) {
            printCoreconf(&val->data.array_value->elements[i]);
            if (i != val->data.array_value->size - 1) printf(", ");
        }
        printf("]");
    } else if (val->type == CORECONF_HASHMAP) {
        printCoreconfMap(val->data.map_value);
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

void addToCoreconfArray(CoreconfValueT* arr, CoreconfValueT* value) {
    if (arr->data.array_value->elements == NULL) {
        arr->data.array_value->elements = malloc(sizeof(CoreconfValueT));
        arr->data.array_value->elements[0] = *value;
    } else {
        size_t newSize = arr->data.array_value->size + 1;
        arr->data.array_value->elements = realloc(arr->data.array_value->elements, newSize * sizeof(CoreconfValueT));
        arr->data.array_value->elements[newSize - 1] = *value;
    }
    arr->data.array_value->size++;
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
    }

    // freeValue is true when the value is not part of an array
    if (freeValue) free(val);

    // Set val as NULL
    val = NULL;
}

// Iterate over CoreconfHashMap and apply a function to each CoreconfObject value
void iterateCoreconfHashMap(CoreconfHashMapT* map, void* udata, void (*f)(CoreconfObjectT* object, void* udata)) {
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        CoreconfObjectT* current = map->table[i];
        while (current != NULL) {
            f(current, udata);
            current = current->next;
        }
    }
}

// Knuth's multiplicative hash for generating hash values for CoreconfHashMap Keys
// TODO Supports only 32-bit hash values, support for SID 64-bit hash values and negative keys
size_t hashKey(uint32_t key) {
    // return (size_t) key % HASHMAP_TABLE_SIZE;
    return (size_t)((key * 2654435769) >> 32) % HASHMAP_TABLE_SIZE;
}

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
