#include "../include/serialization.h"

#include <nanocbor/nanocbor.h>
#include <stdio.h>

#include "../include/sid.h"

/**
 * Internal methods, not exposed to the user
 */
static int parseArray(nanocbor_value_t* value, CoreconfValueT* coreconfValue, unsigned indent);
static int parseMap(nanocbor_value_t* value, CoreconfValueT* coreconfValue, unsigned indent);

// Serialization and Deserialization into CBOR
// Returns 0 on success, -1 on an unsupported value or when the encoder buffer is too small
int coreconfToCBOR(CoreconfValueT* coreconfValue, nanocbor_encoder_t* cbor) {
    if (coreconfValue == NULL) return -1;
    int res = 0;
    switch (coreconfValue->type) {
        case CORECONF_HASHMAP: {
            CoreconfHashMapT* map = coreconfValue->data.map_value;
            res = nanocbor_fmt_map(cbor, map->size);
            for (size_t i = 0; res >= 0 && i < HASHMAP_TABLE_SIZE; i++) {
                for (CoreconfObjectT* object = map->table[i]; res >= 0 && object != NULL; object = object->next) {
                    res = nanocbor_fmt_uint(cbor, object->key);
                    if (res >= 0) res = coreconfToCBOR(object->value, cbor);
                }
            }
            break;
        }
        case CORECONF_ARRAY: {
            size_t arrayLength = coreconfValue->data.array_value->size;
            res = nanocbor_fmt_array(cbor, arrayLength);
            for (size_t i = 0; res >= 0 && i < arrayLength; i++) {
                res = coreconfToCBOR(&coreconfValue->data.array_value->elements[i], cbor);
            }
            break;
        }
        case CORECONF_REAL:
            res = nanocbor_fmt_double(cbor, coreconfValue->data.real_value);
            break;
        case CORECONF_INT_8:
            res = nanocbor_fmt_int(cbor, coreconfValue->data.i8);
            break;
        case CORECONF_INT_16:
            res = nanocbor_fmt_int(cbor, coreconfValue->data.i16);
            break;
        case CORECONF_INT_32:
            res = nanocbor_fmt_int(cbor, coreconfValue->data.i32);
            break;
        case CORECONF_INT_64:
            res = nanocbor_fmt_int(cbor, coreconfValue->data.i64);
            break;
        case CORECONF_UINT_8:
            res = nanocbor_fmt_uint(cbor, coreconfValue->data.u8);
            break;
        case CORECONF_UINT_16:
            res = nanocbor_fmt_uint(cbor, coreconfValue->data.u16);
            break;
        case CORECONF_UINT_32:
            res = nanocbor_fmt_uint(cbor, coreconfValue->data.u32);
            break;
        case CORECONF_UINT_64:
            res = nanocbor_fmt_uint(cbor, coreconfValue->data.u64);
            break;
        case CORECONF_STRING:
            res = nanocbor_put_tstr(cbor, (const char*)coreconfValue->data.string_value);
            break;
        case CORECONF_TRUE:
            res = nanocbor_fmt_bool(cbor, true);
            break;
        case CORECONF_FALSE:
            res = nanocbor_fmt_bool(cbor, false);
            break;
        case CORECONF_NULL:
            res = nanocbor_fmt_null(cbor);
            break;
        case CORECONF_BYTES:
            res = nanocbor_put_bstr(cbor, coreconfValue->data.bytes_value.data,
                                    coreconfValue->data.bytes_value.length);
            break;
        case CORECONF_TAG:
            res = nanocbor_fmt_tag(cbor, coreconfValue->data.tag_value.number);
            if (res >= 0) res = coreconfToCBOR(coreconfValue->data.tag_value.value, cbor);
            break;
        default:
            // Something wrong happened
            return -1;
    }

    return (res < 0) ? -1 : 0;
}

// Deserialization from CBOR to Coreconf
CoreconfValueT* cborToCoreconfValue(nanocbor_value_t* value, unsigned indent) {
    CoreconfValueT* coreconfValue = NULL;
    uint8_t type = nanocbor_get_type(value);
    if (indent > CORECONF_MAX_DEPTH) {
        return NULL;
    }
    int res = 0;
    switch (type) {
        case NANOCBOR_TYPE_UINT: {
            // Use the overflow flag in nanocbor to get integers of the correct length
            uint8_t unsignedInteger8 = 0;
            res = nanocbor_get_uint8(value, &unsignedInteger8);
            if (res >= 0) {
                coreconfValue = createCoreconfUint8(unsignedInteger8);
            } else if (res == NANOCBOR_ERR_OVERFLOW) {
                uint16_t unsignedInteger16 = 0;
                res = nanocbor_get_uint16(value, &unsignedInteger16);
                if (res >= 0) {
                    coreconfValue = createCoreconfUint16(unsignedInteger16);
                } else if (res == NANOCBOR_ERR_OVERFLOW) {
                    uint32_t unsignedInteger32 = 0;
                    res = nanocbor_get_uint32(value, &unsignedInteger32);
                    if (res >= 0) {
                        coreconfValue = createCoreconfUint32(unsignedInteger32);
                    } else if (res == NANOCBOR_ERR_OVERFLOW) {
                        // we have a uint64
                        uint64_t unsignedInteger64 = 0;
                        res = nanocbor_get_uint64(value, &unsignedInteger64);
                        if (res >= 0) {
                            coreconfValue = createCoreconfUint64(unsignedInteger64);
                        }
                    }
                }
            }

        } break;

        case NANOCBOR_TYPE_NINT: {
            // Use the overflow flag in nanocbor to get integers of the correct length
            int8_t signedInteger8 = 0;
            res = nanocbor_get_int8(value, &signedInteger8);
            if (res >= 0) {
                coreconfValue = createCoreconfInt8(signedInteger8);
            } else if (res == NANOCBOR_ERR_OVERFLOW) {
                int16_t signedInteger16 = 0;
                res = nanocbor_get_int16(value, &signedInteger16);
                if (res >= 0) {
                    coreconfValue = createCoreconfInt16(signedInteger16);
                } else if (res == NANOCBOR_ERR_OVERFLOW) {
                    int32_t signedInteger32 = 0;
                    res = nanocbor_get_int32(value, &signedInteger32);
                    if (res >= 0) {
                        coreconfValue = createCoreconfInt32(signedInteger32);
                    } else if (res == NANOCBOR_ERR_OVERFLOW) {
                        // we have a int64
                        int64_t signedInteger64 = 0;
                        res = nanocbor_get_int64(value, &signedInteger64);
                        if (res >= 0) {
                            coreconfValue = createCoreconfInt64(signedInteger64);
                        }
                    }
                }
            }

        } break;
        case NANOCBOR_TYPE_BSTR: {
            // Not NUL-terminated and may contain 0x00: copy exactly `len` bytes
            const uint8_t* buf = NULL;
            size_t len = 0;
            res = nanocbor_get_bstr(value, &buf, &len);
            if (res >= 0) {
                coreconfValue = createCoreconfBytes(buf, len);
            }
        } break;
        case NANOCBOR_TYPE_TSTR: {
            const uint8_t* buf = NULL;
            size_t len = 0;
            res = nanocbor_get_tstr(value, &buf, &len);
            if (res >= 0) {
                coreconfValue = createCoreconfStringLength((const char*)buf, len);
            }
        } break;
        case NANOCBOR_TYPE_TAG: {
            // Keep any tag (RFC 9254 uses 43-47 inside unions) around its decoded value
            uint32_t tagNumber = 0;
            res = nanocbor_get_tag(value, &tagNumber);
            if (res >= 0) {
                CoreconfValueT* tagged = cborToCoreconfValue(value, indent + 1);
                if (tagged == NULL) {
                    return NULL;
                }
                coreconfValue = createCoreconfTag(tagNumber, tagged);
                if (coreconfValue == NULL) {
                    freeCoreconf(tagged, true);
                }
            }
        } break;
        case NANOCBOR_TYPE_ARR: {
            coreconfValue = createCoreconfArray();
            res = parseArray(value, coreconfValue, indent);
            if (res < 0) {
                freeCoreconf(coreconfValue, true);
                return NULL;
            }
        } break;
        case NANOCBOR_TYPE_MAP: {
            coreconfValue = createCoreconfHashmap();
            res = parseMap(value, coreconfValue, indent);
            if (res < 0) {
                freeCoreconf(coreconfValue, true);
                return NULL;
            }
        } break;
        // NOTE: There is no NANOCBOR_TYPE_DOUBLE mask, which is weird?!
        // NANOCBOR_TYPE_FLOAT includes both floating-point numbers and simple values (bool, null, etc.)
        case NANOCBOR_TYPE_FLOAT: {
            // null first (YANG empty, RFC 9254 section 6.9), then boolean, then double
            bool boolValue = false;
            if (nanocbor_get_null(value) >= 0) {
                coreconfValue = createCoreconfNull();
            } else if (nanocbor_get_bool(value, &boolValue) >= 0) {
                coreconfValue = createCoreconfBoolean(boolValue);
            } else {
                // Try double
                double doubleValue = 0;
                res = nanocbor_get_double(value, &doubleValue);
                if (res >= 0) {
                    coreconfValue = createCoreconfReal(doubleValue);
                }
            }
        } break;
        default:
            break;
    }
    return coreconfValue;
}

static int parseArray(nanocbor_value_t* value, CoreconfValueT* coreconfValue, unsigned indent) {
    nanocbor_value_t cborArrayValue;
    if (nanocbor_enter_array(value, &cborArrayValue) < NANOCBOR_OK) {
        printf("Error entering array\n");
        return -1;
    }

    // A definite-length CBOR array states its item count up front: allocate
    // exactly that once, instead of growing per item.  Indefinite-length
    // arrays have no count, so they grow one item at a time.
    CoreconfArrayT* array = coreconfValue->data.array_value;
    bool definite = !nanocbor_container_indefinite(&cborArrayValue);
    size_t count = 0;
    if (definite) {
        count = nanocbor_array_items_remaining(&cborArrayValue);
        // Every item takes at least one byte, so a larger count is malformed input
        if (count > (size_t)(cborArrayValue.end - cborArrayValue.cur)) {
            printf("Error: array length exceeds the input\n");
            return -1;
        }
        if (count > 0) {
            array->elements = malloc(count * sizeof(CoreconfValueT));
            if (array->elements == NULL) {
                return -1;
            }
        }
    }

    while (!nanocbor_at_end(&cborArrayValue)) {
        CoreconfValueT* arrayValue = cborToCoreconfValue(&cborArrayValue, indent + 1);
        if (arrayValue == NULL) {
            printf("Error: cborToCoreconfValue returned NULL in array\n");
            return -1;
        }
        if (definite && array->size < count) {
            array->elements[array->size++] = *arrayValue;
        } else {
            addToCoreconfArray(coreconfValue, arrayValue);
        }
        // The array stores a copy of the value struct; free the now-empty wrapper
        free(arrayValue);
    }
    if (nanocbor_leave_container(value, &cborArrayValue) < 0) {
        printf("Error leaving array container\n");
        return -1;
    }
    return NANOCBOR_OK;
}

static int parseMap(nanocbor_value_t* value, CoreconfValueT* coreconfValue, unsigned indent) {
    nanocbor_value_t map;
    int loopCount = 0;
    if (nanocbor_enter_map(value, &map) < NANOCBOR_OK) {
        printf("Error entering map\n");
        return -1;
    }

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
        CoreconfValueT* mapValue = cborToCoreconfValue(&map, indent + 1);
        if (mapValue == NULL) {
            printf("Error: cborToCoreconfValue returned NULL for map value at key %lu\n", coreconfKey);
            return -1;
        }
        insertCoreconfHashMap(coreconfValue->data.map_value, coreconfKey, mapValue);
        loopCount++;
    }
    if (nanocbor_leave_container(value, &map) < 0) {
        printf("Error leaving map container\n");
        return -1;
    }
    return NANOCBOR_OK;
}

int keyMappingHashMapToCBOR(struct hashmap* keyMappingHashMap, nanocbor_encoder_t* cbor) {
    // Iterate through keyMappingHashMap
    size_t iter = 0;
    void* item;

    // Start map encoding in CBOR
    nanocbor_fmt_map(cbor, hashmap_count(keyMappingHashMap));

    while (hashmap_iter(keyMappingHashMap, &iter, &item)) {
        const KeyMappingT* keyMapping = item;
        // Add items to the map
        nanocbor_fmt_uint(cbor, keyMapping->key);
        // Iterate through the keyMapping->dynamicLongList and add to the array
        nanocbor_fmt_array(cbor, keyMapping->dynamicLongList->size);
        for (size_t i = 0; i < keyMapping->dynamicLongList->size; i++) {
            // Dereference the pointer and add to the array
            uint64_t SIDKey = *(keyMapping->dynamicLongList->longList + i);
            nanocbor_fmt_uint(cbor, SIDKey);
        }
        // End the Array
    }
    // End the map
    return 0;
}

// Deserialize a CBOR buffer to a KeyMappingHashMap
struct hashmap* cborToKeyMappingHashMap(nanocbor_value_t* value) {
    struct hashmap* keyMappingHashMap =
        hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, keyMappingFree, NULL);
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
        KeyMappingT* keyMapping = malloc(sizeof(KeyMappingT));
        keyMapping->key = key;
        keyMapping->dynamicLongList = createDynamicLongList();

        nanocbor_value_t array;
        if (nanocbor_enter_array(&map, &array) < NANOCBOR_OK) {
            freeDynamicLongList(keyMapping->dynamicLongList);
            free(keyMapping);
            return NULL;
        }
        while (!nanocbor_at_end(&array)) {
            // Safety mechanism to avoid infinite loops
            if (loopCounter > CORECONF_MAX_LOOP) {
                freeDynamicLongList(keyMapping->dynamicLongList);
                free(keyMapping);
                return NULL;
            }

            uint64_t SIDKey = 0;
            int res = nanocbor_get_uint64(&array, &SIDKey);
            if (res < 0) {
                printf("Error parsing array value\n");
            }
            // Add to the dynamicLongList
            addLong(keyMapping->dynamicLongList, SIDKey);
            loopCounter++;
        }
        nanocbor_leave_container(&map, &array);

        // Insert into the hashmap: hashmap_set copies the struct, the map now owns the list
        hashmap_set(keyMappingHashMap, keyMapping);
        free(keyMapping);
        loopCounter++;
    }
    nanocbor_leave_container(value, &map);
    return keyMappingHashMap;
}
