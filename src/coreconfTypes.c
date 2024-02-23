#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <nanocbor/nanocbor.h>
#include "../include/coreconfTypes.h"

int _parse_array(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent);
int _parse_map(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent);
int _parse_float(nanocbor_value_t *value);


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

void serializeCoreconfObject(CoreconfObjectT* object, void* cbor_) {
    nanocbor_encoder_t *cbor = (nanocbor_encoder_t *) cbor_;
    nanocbor_fmt_uint(cbor, object->key);
    coreconfToCBOR(object->value, cbor);
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


// Serialization and Deserialization into CBOR
int coreconfToCBOR(CoreconfValueT* coreconfValue, nanocbor_encoder_t *cbor){
    switch (coreconfValue->type){
        case CORECONF_HASHMAP:{
            nanocbor_fmt_map(cbor, coreconfValue->data.map_value->size);
            iterateCoreconfHashMap(coreconfValue->data.map_value, (void*) cbor, serializeCoreconfObject);
            //nanocbor_fmt_end_indefinite(cbor);
            break;
        }
        case CORECONF_ARRAY:
            nanocbor_fmt_array(cbor, coreconfValue->data.array_value->size);
            for (size_t i = 0; i < coreconfValue->data.array_value->size; i++) {
                coreconfToCBOR(&coreconfValue->data.array_value->elements[i], cbor);
            }
            //nanocbor_fmt_end_indefinite(cbor);
            break;
        case CORECONF_REAL:
            nanocbor_fmt_double(cbor, coreconfValue->data.real_value);
            break;
        case CORECONF_INTEGER:
            nanocbor_fmt_uint(cbor, coreconfValue->data.integer_value);
            break;
        case CORECONF_STRING:
            // Null terminate string value and then put it
            nanocbor_put_tstr(cbor, (const char*) coreconfValue->data.string_value);
            break;
        case CORECONF_TRUE:
            nanocbor_fmt_bool(cbor, 1);
            break;
        case CORECONF_FALSE:
            nanocbor_fmt_bool(cbor, 0);
            break;
        default:
            break;
    }
}


// Deserialization from CBOR to Coreconf
CoreconfValueT* cborToCoreconfValue(nanocbor_value_t *value, unsigned indent){
    CoreconfValueT* coreconfValue = NULL;
    uint8_t type = nanocbor_get_type(value);
    if (indent > CORECONF_MAX_DEPTH) {
        return NULL;
    }
    int res = 0;
    switch (type) {
    case NANOCBOR_TYPE_UINT: {
        uint64_t uint = 0;
        res = nanocbor_get_uint64(value, &uint);
        if (res >= 0) {
            coreconfValue = createCoreconfReal(uint);
        }
    } break;
    case NANOCBOR_TYPE_NINT: {
        int64_t nint = 0;
        res = nanocbor_get_int64(value, &nint);
        if (res >= 0) {
            coreconfValue = createCoreconfReal(nint);
        }
    } break;
    case NANOCBOR_TYPE_BSTR: {
        const uint8_t *buf = NULL;
        size_t len = 0;
        res = nanocbor_get_bstr(value, &buf, &len);
        if (res >= 0) {
            if (!buf) {
                return NULL;
            }
            coreconfValue = createCoreconfString((const char *)buf);
        }
    } break;
    case NANOCBOR_TYPE_TSTR: {
        const uint8_t *buf = NULL;
        size_t len = 0;
        res = nanocbor_get_tstr(value, &buf, &len);

        char formattedString[len+1];
        // Copy the source string into the destination string using snprintf
        snprintf(formattedString, (int) len+1, "%.*s", (int) len, buf);

        if (res >= 0) {
            coreconfValue = createCoreconfString((const char *)formattedString);
        }
    } break;
    case NANOCBOR_TYPE_ARR: {
        coreconfValue = createCoreconfArray();
        res = _parse_array(value, coreconfValue, indent);
    } break;
    case NANOCBOR_TYPE_MAP: {
        coreconfValue = createCoreconfHashmap();
        res = _parse_map(value, coreconfValue, indent);
    } break;
    case NANOCBOR_TYPE_FLOAT: {
        double doubleValue = 0;
        res = nanocbor_get_double(value, &doubleValue);
        coreconfValue = createCoreconfReal(doubleValue);
    } break;
    // TODO Future Custom TAGS for Coreconf
    default:
        break;
    }
    return coreconfValue;
}

int _parse_array(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent) {
    nanocbor_value_t cborArrayValue;
    if (nanocbor_enter_array(value, &cborArrayValue) < NANOCBOR_OK) 
        return -1;
    while(!nanocbor_at_end(&cborArrayValue)){
        CoreconfValueT* arrayValue = cborToCoreconfValue(&cborArrayValue, indent + 1);
        addToCoreconfArray(coreconfValue, arrayValue);
    }
    nanocbor_leave_container(value, &cborArrayValue);
    return NANOCBOR_OK;
}


int _parse_map(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent) {
    nanocbor_value_t map;
    if (nanocbor_enter_map(value, &map) < NANOCBOR_OK) 
        return -1;
    
    // Get items in the map
    //int itemSize = nanocbor_map_size(value);

    // Iterate over the map
    while (!nanocbor_at_end(&map)) {
        uint64_t coreconfKey = 0;
        uint64_t res = nanocbor_get_uint64(&map, &coreconfKey);
        if (res < 0) {
            printf("Error parsing map key\n");
            break;
        }
        insertCoreconfHashMap(coreconfValue->data.map_value, 
        coreconfKey, cborToCoreconfValue(&map, indent + 1));
    }
    nanocbor_leave_container(value, &map);
    return NANOCBOR_OK;
}


// Parse float
int _parse_float(nanocbor_value_t *value) {
    double f = 0;
    int res = nanocbor_get_double(value, &f);
    if (res >= NANOCBOR_OK) {
        printf("%f", f);
    }
    return res;
}

