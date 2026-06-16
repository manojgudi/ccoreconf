
#include <nanocbor/nanocbor.h>
#include <stdio.h>

#include "../include/coreconfTypes.h"
#include "hashmap.h"

void serializeCoreconfObject(CoreconfObjectT* object, void* cbor_);
int coreconfToCBOR(CoreconfValueT* coreconfValue, nanocbor_encoder_t* cbor);
CoreconfValueT* cborToCoreconfValue(nanocbor_value_t* value, unsigned indent);

// Serialize KeyMappingHashMap to CBOR
int keyMappingHashMapToCBOR(struct hashmap* keyMappingHashMap, nanocbor_encoder_t* cbor);
// Deserialize KeyMappingHashMap from CBOR
struct hashmap* cborToKeyMappingHashMap(nanocbor_value_t* value);
