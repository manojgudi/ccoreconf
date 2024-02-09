#include <stdio.h>
#include <jansson.h>
#include <nanocbor/nanocbor.h>

// Serialize a generic JSON object to a CBOR buffer
int json_to_cbor(json_t *json, nanocbor_encoder_t *cbor);