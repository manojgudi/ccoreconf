#include <inttypes.h>
#include <nanocbor/nanocbor.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/serialization.h"

int main() {
    CoreconfValueT* arr_val = createCoreconfArray();
    CoreconfValueT* arr_val2 = createCoreconfArray();
    CoreconfValueT* internalMapValue = createCoreconfHashmap();
    CoreconfHashMapT* internalMap = internalMapValue->data.map_value;
    insertCoreconfHashMap(internalMap, (uint64_t)1, createCoreconfString("Internal Map"));

    // Example to create, add and delete coreconfMap
    // NOTE coreconfMap will ONLY contain an unsigned 64-bit integer as key
    CoreconfValueT* mapValue = createCoreconfHashmap();
    CoreconfHashMapT* map = mapValue->data.map_value;

    insertCoreconfHashMap(map, (uint64_t)1, createCoreconfString("John Doe"));
    insertCoreconfHashMap(map, (uint64_t)2, createCoreconfReal(301.12));
    insertCoreconfHashMap(map, (uint64_t)3, createCoreconfString("Jane Doe"));
    insertCoreconfHashMap(map, (uint64_t)103, createCoreconfString("Jane Doe"));
    insertCoreconfHashMap(map, (uint64_t)1, createCoreconfReal(-25.25));
    insertCoreconfHashMap(map, (uint64_t)1292, internalMapValue);

    addToCoreconfArray(arr_val, createCoreconfInt8(1));
    addToCoreconfArray(arr_val, createCoreconfInt64(-22));
    addToCoreconfArray(arr_val, createCoreconfUint64(3));
    printf("Array value: ");
    printCoreconf(arr_val);
    printf("\n");

    addToCoreconfArray(arr_val2, createCoreconfString("Jakarta"));
    addToCoreconfArray(arr_val2, createCoreconfString("Diu"));
    addToCoreconfArray(arr_val2, createCoreconfString("Rennes"));
    printf("Array value: ");
    printCoreconf(arr_val);
    printf("\n");

    insertCoreconfHashMap(map, (uint64_t)4, arr_val);
    insertCoreconfHashMap(map, (uint64_t)104, arr_val);
    insertCoreconfHashMap(map, (uint64_t)204, arr_val2);
    addToCoreconfArray(arr_val, createCoreconfInt8(4));
    printf("Map value: \n");
    // Create new CORECONF value

    printCoreconf(mapValue);

    size_t cbor_buffer_size = 5024;  // Adjust as needed
    uint8_t cbor_buffer[cbor_buffer_size];

    // Coreconf to CBOR
    nanocbor_encoder_t encoder;
    nanocbor_encoder_init(&encoder, cbor_buffer, cbor_buffer_size);
    coreconfToCBOR(mapValue, &encoder);
    // coreconfToCBOR(arr_val2, &encoder);
    printf("CBOR:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&encoder); i++) {
        printf("%02x ", cbor_buffer[i]);
    }
    printf("\n");

    // CBOR to Coreconf
    nanocbor_value_t decoder;
    nanocbor_decoder_init(&decoder, cbor_buffer, cbor_buffer_size);
    CoreconfValueT* coreconfValueDeserialized = cborToCoreconfValue(&decoder, 0);
    printf("\nDeserialized Coreconf: \n");
    printCoreconf(coreconfValueDeserialized);
    printf("\n");

    // Careful with Reference counting of arr_val
    freeCoreconf(mapValue, true);
    freeCoreconf(arr_val, false);
    freeCoreconf(arr_val2, false);
    freeCoreconf(coreconfValueDeserialized, true);

    return 0;
}
