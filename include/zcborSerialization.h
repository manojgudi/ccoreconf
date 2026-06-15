#include <stdio.h>
#include <zcbor_common.h>
#include <zcbor_encode.h>

#include "../include/coreconfTypes.h"

void coreconfToCBOR_(CoreconfValueT *coreconfValue, zcbor_state_t *encoder);
void serializeCoreconfObject_(CoreconfObjectT *object, void *cbor_);

CoreconfValueT *cborToCoreconfValue_(const uint8_t *cborData, zcbor_state_t *decoder);