#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "../include/coreconfTypes.h"



CoreconfValueT* createCoreconfString(const char* value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_STRING;
    val->data.string_value = strdup(value);
    return val;
}

CoreconfValueT* createCoreconfReal(double value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_REAL;
    val->data.real_value = value;
    return val;
}

CoreconfValueT* createCoreconfInteger(uint64_t value) {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_INTEGER;
    val->data.integer_value = value;
    return val;
}

CoreconfObjectT* createCoreconfObject() {
    CoreconfObjectT* obj = malloc(sizeof(CoreconfObjectT));
    obj->key = 0;
    obj->value = NULL;
    obj->next = NULL;
    return obj;
}

CoreconfValueT* createCoreconfHashmap() {
    CoreconfValueT* val = malloc(sizeof(CoreconfValueT));
    val->type = CORECONF_HASHMAP;
    val->data.map_value = malloc(sizeof(CoreconfHashMapT));
    val->data.map_value->size = 0;
    val->data.map_value->capacity = HASHMAP_TABLE_SIZE;
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; ++i) {
        val->data.map_value->table[i] = NULL;
    }
    return val;
} 


// Insert Coreconf Object into CoreconfHashMap
int insertCoreconfHashMap(CoreconfHashMapT* map, uint64_t key, CoreconfValueT* value) {
    size_t index = hashKey(key);

    CoreconfObjectT* coreconfObject_ = malloc(sizeof(CoreconfObjectT));
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
        while(current != NULL){
            if (current->key == key){
                freeCoreconf(current->value, true);
                current->value = value;
                free(coreconfObject_);
                return 0;
            } 
            current = current->next;
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
    size_t index = hashKey( (uint32_t) key);
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
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; ++i) {
        CoreconfObjectT* current = map->table[i];
        while (current != NULL) {
            CoreconfObjectT* next = current->next;
            // Free the value only if its not null
            if (current->value != NULL)
                freeCoreconf(current->value, true);

            free(current);
            current = next;
        }
    }
    free(map);
}

void printCoreconfMap(CoreconfHashMapT* map) {
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; ++i) {
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
    if (!obj)
        return;
    printf("Key: %" PRIu64 " Value: ", obj->key);
    printCoreconf(obj->value);
    printf("\n");
}

void printCoreconf(CoreconfValueT* val) {
    if (!val)
        return;
    if (val->type == CORECONF_STRING)
        printf("%s", val->data.string_value);
    else if (val->type == CORECONF_REAL)
        printf("%f", val->data.real_value);
    else if (val->type == CORECONF_INTEGER)
        printf("%" PRIu64, val->data.integer_value);
     else if (val->type == CORECONF_ARRAY) {
        printf("[");
        for (size_t i = 0; i < val->data.array_value->size; ++i) {
            printCoreconf(&val->data.array_value->elements[i]);
            if (i != val->data.array_value->size - 1)
                printf(", ");
        }
        printf("]");
    } else if (val->type == CORECONF_HASHMAP){
        printCoreconfMap(val->data.map_value);
    }
}

CoreconfValueT* createCoreconfArray() {
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
    if (!val)
        return;
    if (val->type == CORECONF_STRING)
        free(val->data.string_value);
     else if (val->type == CORECONF_ARRAY) {
        for (size_t i = 0; i < val->data.array_value->size ; i++) {
            freeCoreconf(&val->data.array_value->elements[i], false);
        }
        free(val->data.array_value->elements);
        free(val->data.array_value);
    } else if (val->type == CORECONF_HASHMAP) {
        freeCoreconfHashMap(val->data.map_value);
    }

    // freeValue is true when the value is not part of an array 
    if (freeValue)
        free(val);

    // Set val as NULL
    val = NULL;
}

// Iterate over CoreconfHashMap and apply a function to each CoreconfObject value
void iterateCoreconfHashMap(CoreconfHashMapT* map, void *udata, void (*f) (CoreconfObjectT *object, void *udata)){
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; ++i) {
        CoreconfObjectT* current = map->table[i];
        while (current != NULL) {
            f(current, udata);
            current = current->next;
        }
    }
}



// Knuth's multiplicative hash for generating hash values for CoreconfHashMap Keys
// TODO Supports only 32-bit hash values, support for SID 64-bit hash values and negative keys
size_t  hashKey(uint32_t key) {
    //return (size_t) key % HASHMAP_TABLE_SIZE;
    return (size_t) ((key * 2654435769) >> 32) % HASHMAP_TABLE_SIZE;
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


