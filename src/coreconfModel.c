/*
 * src/coreconfModel.c — CoreconfModelT lifecycle and model-aware
 * wrapper functions.
 *
 * The functions in this file are one-line passthrough wrappers around
 * the legacy APIs in coreconfManipulation.c.  Their only purpose is to
 * give multi-model code a single handle to pass around instead of
 * (root, clookup_hashmap, keymap_hashmap) tuples.
 *
 * The legacy single-model APIs are untouched, so existing call sites
 * (notably examples/demo_functionalities_coreconf.c) keep working.
 */

#include "../include/coreconfModel.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/coreconfManipulation.h"
#include "../include/coreconfTypes.h"
#include "../include/hashmap.h"
#include "../include/serialization.h"

/* ------------------------------------------------------------------------- *
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

CoreconfModelT *ccoreconf_model_load(const uint8_t *instance_cbor,
                                     size_t instance_len,
                                     const uint8_t *keymap_cbor,
                                     size_t keymap_len) {
    if (instance_cbor == NULL || keymap_cbor == NULL) {
        return NULL;
    }

    CoreconfModelT *model = (CoreconfModelT *)calloc(1, sizeof(CoreconfModelT));
    if (model == NULL) {
        return NULL;
    }

    /* Decode the instance tree. */
    nanocbor_value_t instance_decoder;
    nanocbor_decoder_init(&instance_decoder, instance_cbor, instance_len);
    model->root = cborToCoreconfValue(&instance_decoder, 0);
    if (model->root == NULL) {
        free(model);
        return NULL;
    }

    /* Decode the key-mapping table. */
    nanocbor_value_t keymap_decoder;
    nanocbor_decoder_init(&keymap_decoder, keymap_cbor, keymap_len);
    model->keymap_hashmap = cborToKeyMappingHashMap(&keymap_decoder);
    if (model->keymap_hashmap == NULL) {
        freeCoreconf(model->root, true);
        free(model);
        return NULL;
    }

    /* Build the clookup table. */
    model->clookup_hashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                        clookupHash, clookupCompare, NULL, NULL);
    if (model->clookup_hashmap == NULL) {
        hashmap_free(model->keymap_hashmap);
        freeCoreconf(model->root, true);
        free(model);
        return NULL;
    }
    buildCLookupHashmapFromCoreconf(model->root, model->clookup_hashmap, 0, 0);

    return model;
}

void ccoreconf_model_free(CoreconfModelT *model) {
    if (model == NULL) {
        return;
    }
    if (model->clookup_hashmap != NULL) {
        freeCLookupHashmap(model->clookup_hashmap);
        hashmap_free(model->clookup_hashmap);
        model->clookup_hashmap = NULL;
    }
    if (model->keymap_hashmap != NULL) {
        hashmap_free(model->keymap_hashmap);
        model->keymap_hashmap = NULL;
    }
    if (model->root != NULL) {
        freeCoreconf(model->root, true);
        model->root = NULL;
    }
    free(model);
}

/* ------------------------------------------------------------------------- *
 * Model-aware query wrappers                                                *
 * ------------------------------------------------------------------------- */

PathNodeT *findRequirementForSID_model(CoreconfModelT *model, uint64_t sid) {
    if (model == NULL || model->clookup_hashmap == NULL || model->keymap_hashmap == NULL) {
        return NULL;
    }
    return findRequirementForSID(sid, model->clookup_hashmap, model->keymap_hashmap);
}

CoreconfValueT *examineCoreconfValue_model(CoreconfModelT *model,
                                           DynamicLongListT *requestKeys,
                                           PathNodeT *headNode) {
    if (model == NULL || model->root == NULL) {
        return NULL;
    }
    return examineCoreconfValue(model->root, requestKeys, headNode);
}

void buildCLookupHashmapFromCoreconf_model(CoreconfModelT *model) {
    if (model == NULL || model->root == NULL) {
        return;
    }
    /* Free any pre-existing clookup entries before rebuilding. */
    if (model->clookup_hashmap != NULL) {
        freeCLookupHashmap(model->clookup_hashmap);
        hashmap_clear(model->clookup_hashmap, false);
    } else {
        model->clookup_hashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                            clookupHash, clookupCompare, NULL, NULL);
        if (model->clookup_hashmap == NULL) {
            return;
        }
    }
    buildCLookupHashmapFromCoreconf(model->root, model->clookup_hashmap, 0, 0);
}