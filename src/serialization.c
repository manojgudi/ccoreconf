#include "../include/serialization.h"

#include <jansson.h>
#include <nanocbor/nanocbor.h>
#include <stdio.h>

#include "../include/sid.h"

/**
 * Internal methods, not exposed to the user
 */
int _parse_array(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent);
int _parse_map(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent);
int _parse_float(nanocbor_value_t *value);

void serializeCoreconfObject(CoreconfObjectT *object, void *cbor_) {
    nanocbor_encoder_t *cbor = (nanocbor_encoder_t *)cbor_;
    nanocbor_fmt_uint(cbor, object->key);
    coreconfToCBOR(object->value, cbor);
}

// Convert a generic json object to cbor
int json_to_cbor(json_t *json, nanocbor_encoder_t *cbor) {
    if (json_is_object(json)) {
        nanocbor_fmt_map(cbor, json_object_size(json));
        const char *key;
        json_t *value;
        json_object_foreach(json, key, value) {
            nanocbor_fmt_int(cbor, atoi(key));
            json_to_cbor(value, cbor);
        }
        nanocbor_fmt_end_indefinite(cbor);
    } else if (json_is_array(json)) {
        nanocbor_fmt_array(cbor, json_array_size(json));
        for (size_t i = 0; i < json_array_size(json); i++) {
            json_to_cbor(json_array_get(json, i), cbor);
        }
        nanocbor_fmt_end_indefinite(cbor);
    } else if (json_is_string(json)) {
        nanocbor_put_bstr(cbor, (const uint8_t *)json_string_value(json), strlen(json_string_value(json)));
    } else if (json_is_integer(json)) {
        nanocbor_fmt_int(cbor, json_integer_value(json));
    } else if (json_is_real(json)) {
        nanocbor_fmt_float(cbor, json_real_value(json));
    } else if (json_is_true(json)) {
        nanocbor_fmt_bool(cbor, 1);
    } else if (json_is_false(json)) {
        nanocbor_fmt_bool(cbor, 0);
    } else if (json_is_null(json)) {
        nanocbor_fmt_null(cbor);
    } else {
        return -1;
    }
    return 0;
}

// Serialization and Deserialization into CBOR
int coreconfToCBOR(CoreconfValueT *coreconfValue, nanocbor_encoder_t *cbor) {
    switch (coreconfValue->type) {
        case CORECONF_HASHMAP: {
            nanocbor_fmt_map(cbor, coreconfValue->data.map_value->size);
            iterateCoreconfHashMap(coreconfValue->data.map_value, (void *)cbor, serializeCoreconfObject);
            // nanocbor_fmt_end_indefinite(cbor);
            break;
        }
        case CORECONF_ARRAY: {
            size_t arrayLength = coreconfValue->data.array_value->size;
            nanocbor_fmt_array(cbor, arrayLength);
            for (size_t i = 0; i < arrayLength; i++) {
                coreconfToCBOR(&coreconfValue->data.array_value->elements[i], cbor);
            }
        }
        // nanocbor_fmt_end_indefinite(cbor);
        break;
        case CORECONF_REAL:
            nanocbor_fmt_double(cbor, coreconfValue->data.real_value);
            break;
        case CORECONF_INTEGER:
            nanocbor_fmt_uint(cbor, coreconfValue->data.integer_value);
            break;
        case CORECONF_STRING:
            // Null terminate string value and then put it
            nanocbor_put_tstr(cbor, (const char *)coreconfValue->data.string_value);
            break;
        case CORECONF_TRUE:
            nanocbor_fmt_uint(cbor, 1);
            break;
        case CORECONF_FALSE:
            nanocbor_fmt_uint(cbor, 0);
            break;
        default:
            // Something wrong happened
            return -1;
    }

    return 0;
}

// Deserialization from CBOR to Coreconf
CoreconfValueT *cborToCoreconfValue(nanocbor_value_t *value, unsigned indent) {
    CoreconfValueT *coreconfValue = NULL;
    uint8_t type = nanocbor_get_type(value);
    if (indent > CORECONF_MAX_DEPTH) {
        return NULL;
    }
    int res = 0;
    switch (type) {
        case NANOCBOR_TYPE_UINT: {
            uint64_t unsignedInteger = 0;
            res = nanocbor_get_uint64(value, &unsignedInteger);
            if (res >= 0) {
                coreconfValue = createCoreconfInteger(unsignedInteger);
            }
        } break;
        case NANOCBOR_TYPE_NINT: {
            int64_t nint = 0;
            res = nanocbor_get_int64(value, &nint);
            if (res >= 0) {
                coreconfValue = createCoreconfInteger(nint);
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

            char formattedString[len + 1];
            // Copy the source string into the destination string using snprintf
            snprintf(formattedString, (int)len + 1, "%.*s", (int)len, buf);

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
            float floatValue = 0;
            res = nanocbor_get_float(value, &floatValue);
            coreconfValue = createCoreconfReal(floatValue);
        } break;
        // TODO Future Custom TAGS for Coreconf
        default:
            break;
    }
    return coreconfValue;
}

int _parse_array(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent) {
    nanocbor_value_t cborArrayValue;
    if (nanocbor_enter_array(value, &cborArrayValue) < NANOCBOR_OK) {
        printf("Error entering array\n");
        return -1;
    }
    while (!nanocbor_at_end(&cborArrayValue)) {
        CoreconfValueT *arrayValue = cborToCoreconfValue(&cborArrayValue, indent + 1);
        addToCoreconfArray(coreconfValue, arrayValue);
    }
    nanocbor_leave_container(value, &cborArrayValue);
    return NANOCBOR_OK;
}

int _parse_map(nanocbor_value_t *value, CoreconfValueT *coreconfValue, unsigned indent) {
    nanocbor_value_t map;
    int loopCount = 0;
    if (nanocbor_enter_map(value, &map) < NANOCBOR_OK) {
        printf("Error entering map\n");
        return -1;
    }

    // Get items in the map
    // int itemSize = nanocbor_map_size(value);

    // Iterate over the map
    while (!nanocbor_at_end(&map)) {
        if (loopCount > CORECONF_MAX_LOOP) return -1;

        uint64_t coreconfKey = 0;
        int res = nanocbor_get_uint64(&map, &coreconfKey);
        if (res < 0) {
            printf("Error parsing map key\n");
            nanocbor_skip(value);
            return -2;
        }
        insertCoreconfHashMap(coreconfValue->data.map_value, coreconfKey, cborToCoreconfValue(&map, indent + 1));
        loopCount++;
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

// Map a primitive JSON value to a CORECONF Value
CoreconfValueT *mapJSONToCoreconfValue(json_t *jsonValue) {
    // Find type of json Object
    if (json_is_string(jsonValue)) {
        return createCoreconfString(json_string_value(jsonValue));
    } else if (json_is_integer(jsonValue)) {
        return createCoreconfInteger(json_integer_value(jsonValue));
    } else if (json_is_real(jsonValue)) {
        return createCoreconfReal(json_real_value(jsonValue));
    } else if (json_is_true(jsonValue)) {
        return createCoreconfBoolean(true);
    } else if (json_is_false(jsonValue)) {
        return createCoreconfBoolean(false);
    } else {
        // JSON dump the jsonValue and return a CoreconfString value
        char *jsonString = json_dumps(jsonValue, JSON_ENCODE_ANY);
        return createCoreconfString(jsonString);
    }
}

int keyMappingHashMapToCBOR(struct hashmap *keyMappingHashMap, nanocbor_encoder_t *cbor) {
    // Iterate through keyMappingHashMap
    size_t iter = 0;
    void *item;

    // Start map encoding in CBOR
    nanocbor_fmt_map(cbor, hashmap_count(keyMappingHashMap));

    while (hashmap_iter(keyMappingHashMap, &iter, &item)) {
        const KeyMappingT *keyMapping = item;
        // Add items to the map
        nanocbor_fmt_uint(cbor, keyMapping->key);
        // Iterate through the keyMapping->dynamicLongList and add to the array
        nanocbor_fmt_array(cbor, keyMapping->dynamicLongList->size);
        for (size_t i = 0; i < keyMapping->dynamicLongList->size; i++) {
            // Dereference the pointer and add to the array
            uint64_t sidKey = *(keyMapping->dynamicLongList->longList + i);
            nanocbor_fmt_uint(cbor, sidKey);
        }
        // End the Array
    }
    // End the map
    return 0;
}

// Deserialize a CBOR buffer to a KeyMappingHashMap
struct hashmap *cborToKeyMappingHashMap(nanocbor_value_t *value) {
    struct hashmap *keyMappingHashMap =
        hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
    nanocbor_value_t map;
    if (nanocbor_enter_map(value, &map) < NANOCBOR_OK) return NULL;
    int loopCounter = 0;

    while (!nanocbor_at_end(&map)) {
        // Safety mechanism to avoid infinite loops
        if (loopCounter > CORECONF_MAX_LOOP) return NULL;

        uint64_t key = 0;
        int res = nanocbor_get_uint64(&map, &key);
        if (res < 0) {
            printf("Error parsing map key\n");
        }
        KeyMappingT *keyMapping = malloc(sizeof(KeyMappingT));
        keyMapping->key = key;
        keyMapping->dynamicLongList = malloc(sizeof(DynamicLongListT));
        initializeDynamicLongList(keyMapping->dynamicLongList);

        nanocbor_value_t array;
        if (nanocbor_enter_array(&map, &array) < NANOCBOR_OK) return NULL;
        while (!nanocbor_at_end(&array)) {
            // Safety mechanism to avoid infinite loops
            if (loopCounter > CORECONF_MAX_LOOP) return NULL;

            uint64_t sidKey = 0;
            int res = nanocbor_get_uint64(&array, &sidKey);
            if (res < 0) {
                printf("Error parsing array value\n");
            }
            // Add to the dynamicLongList
            addLong(keyMapping->dynamicLongList, sidKey);
            loopCounter++;
        }
        nanocbor_leave_container(&map, &array);

        // Insert into the hashmap
        hashmap_set(keyMappingHashMap, keyMapping);
        loopCounter++;
    }
    nanocbor_leave_container(value, &map);
    return keyMappingHashMap;
}