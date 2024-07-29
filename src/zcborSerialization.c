#include "../include/zcborSerialization.h"

#include <stdio.h>
#include <string.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>

#include "../include/sid.h"

void serializeCoreconfObject_(CoreconfObjectT *object, void *cbor_) {
    zcbor_state_t *cbor = (zcbor_state_t *)cbor_;
    zcbor_uint64_encode(cbor, &object->key);
    coreconfToCBOR_(object->value, cbor);
}

// Convert a coreconfType to CBOR encoded data to be sent on the wire
void coreconfToCBOR_(CoreconfValueT *coreconfValue, zcbor_state_t *encoder) {
    bool success = false;
    switch (coreconfValue->type) {
        case CORECONF_HASHMAP: {
            success = zcbor_map_start_encode(encoder, coreconfValue->data.map_value->size);
            iterateCoreconfHashMap(coreconfValue->data.map_value, (void *)encoder, serializeCoreconfObject_);
            success &= zcbor_map_end_encode(encoder, coreconfValue->data.map_value->size);
            break;
        }
        case CORECONF_ARRAY: {
            size_t arrayLength = coreconfValue->data.array_value->size;
            success = zcbor_list_start_encode(encoder, arrayLength);
            for (size_t i = 0; i < arrayLength; i++) {
                coreconfToCBOR_(coreconfValue->data.array_value->elements + i, encoder);
            }
            success &= zcbor_list_end_encode(encoder, arrayLength);
            break;
        }

        case CORECONF_REAL:
            success = zcbor_float64_encode(encoder, (const double *)&coreconfValue->data.real_value);
            break;

        case CORECONF_INT_8:
            success = zcbor_int_encode(encoder, &coreconfValue->data.i8, 8);
            break;
        case CORECONF_INT_16:
            success = zcbor_int_encode(encoder, &coreconfValue->data.i16, 16);
            break;
        case CORECONF_INT_32:
            success = zcbor_int32_encode(encoder, &coreconfValue->data.i32);
            break;
        case CORECONF_INT_64:
            success = zcbor_int64_encode(encoder, &coreconfValue->data.i64);
            break;
        case CORECONF_UINT_8:
            success = zcbor_uint_encode(encoder, &coreconfValue->data.u8, 8);
            break;
        case CORECONF_UINT_16:
            success = zcbor_uint_encode(encoder, &coreconfValue->data.u16, 16);
            break;
        case CORECONF_UINT_32:
            success = zcbor_uint32_encode(encoder, &coreconfValue->data.u32);
            break;
        case CORECONF_UINT_64:
            success = zcbor_uint64_encode(encoder, &coreconfValue->data.u64);
            break;
        case CORECONF_TRUE: {
            bool trueValue = true;
            success = zcbor_bool_encode(encoder, &trueValue);
            break;
        }
        case CORECONF_FALSE: {
            bool falseValue = false;
            success = zcbor_bool_encode(encoder, &falseValue);
            break;
        }
        case CORECONF_STRING: {
            const struct zcbor_string zstring = {.value = (const uint8_t *)coreconfValue->data.string_value,
                                                 .len = strlen(coreconfValue->data.string_value)};
            success = zcbor_tstr_encode(encoder, &zstring);
            break;
        }
        default:
            fprintf(stderr, "Unsupported CoreconfValue type.\n");
            break;
    }

    if (!success) {
        fprintf(stderr, "Failed to encode CoreconfValue.\n");
    }
}

// Convert a CBOR encoded data received from the wire to CoreconfValue type for internal processing
// Stuck :(
CoreconfValueT *cborToCoreconfValue_(const uint8_t *cborData, zcbor_state_t *decoder) {
    CoreconfValueT *coreconfValue = NULL;

    switch (ZCBOR_MAJOR_TYPE(*decoder->payload)) {
        case ZCBOR_MAJOR_TYPE_PINT: {
            // Unsigned integer
            uint64_t unsignedInteger = 0;
            // NOTE but what if its not 64 bit uint?
            if (zcbor_uint_decode(decoder, &unsignedInteger, 64)) {
                coreconfValue = createCoreconfUint64(unsignedInteger);
            }
            break;
        }

        default:
            break;
    }

    return coreconfValue;
}