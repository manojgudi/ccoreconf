#include <jansson.h>
#include <nanocbor/nanocbor.h>
#include <stdio.h>

#include "../include/coreconfTypes.h"
#include "hashmap.h"

// Serialize a generic JSON object to a CBOR buffer
int json_to_cbor(json_t* json, nanocbor_encoder_t* cbor);

void serializeCoreconfObject(CoreconfObjectT* object, void* cbor_);
int coreconfToCBOR(CoreconfValueT* coreconfValue, nanocbor_encoder_t* cbor);
CoreconfValueT* cborToCoreconfValue(nanocbor_value_t* value, unsigned indent);

// Map a JSON object to a CORECONF Object
CoreconfValueT* mapJSONToCoreconfValue(json_t* jsonValue);

// Serialize KeyMappingHashMap to CBOR
int keyMappingHashMapToCBOR(struct hashmap* keyMappingHashMap, nanocbor_encoder_t* cbor);
// Deserialize KeyMappingHashMap from CBOR
struct hashmap* cborToKeyMappingHashMap(nanocbor_value_t* value);
