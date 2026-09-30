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

/* ---- internal: build hashmap from a static SidKeyMappingT[] table ---- */
static struct hashmap *buildKeymapFromStatic(const SidKeyMappingT *tbl,
                                              size_t count) {
    struct hashmap *km = hashmap_new(sizeof(KeyMappingT), 0, 0, 0,
                                      keyMappingHash, keyMappingCompare,
                                      keyMappingFree, NULL);
    if (km == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        KeyMappingT entry;
        entry.key = (int64_t)tbl[i].list_sid;
        entry.dynamicLongList = createDynamicLongList();
        if (entry.dynamicLongList == NULL) {
            hashmap_free(km);
            return NULL;
        }
        for (size_t k = 0; k < tbl[i].key_sids_count; k++) {
            addLong(entry.dynamicLongList, tbl[i].key_sids[k]);
        }
        hashmap_set(km, &entry);
    }
    return km;
}

/* ---- internal: build hashmap from a static CLookupEntryT[] table ---- */
static struct hashmap *buildClookupFromStatic(const CLookupEntryT *tbl,
                                               size_t count) {
    struct hashmap *cm = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                      clookupHash, clookupCompare,
                                      clookupFree, NULL);
    if (cm == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        CLookupT entry;
        entry.childSID = (int64_t)tbl[i].child_sid;
        entry.dynamicLongList = createDynamicLongList();
        if (entry.dynamicLongList == NULL) {
            hashmap_free(cm);
            return NULL;
        }
        for (size_t p = 0; p < tbl[i].parent_sids_count; p++) {
            addUniqueLong(entry.dynamicLongList, tbl[i].parent_sids[p]);
        }
        hashmap_set(cm, &entry);
    }
    return cm;
}

/* ------------------------------------------------------------------------- *
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

CoreconfModelT *ccoreconfModelLoadDesc(const CoreconfModelDesc *desc) {
    if (desc == NULL || desc->instance_cbor == NULL) {
        return NULL;
    }

    CoreconfModelT *model = (CoreconfModelT *)calloc(1, sizeof(CoreconfModelT));
    if (model == NULL) {
        return NULL;
    }

    /* 1) Decode the instance tree (always required — it's the wire payload). */
    nanocbor_value_t instance_decoder;
    nanocbor_decoder_init(&instance_decoder, desc->instance_cbor, desc->instance_cbor_len);
    model->root = cborToCoreconfValue(&instance_decoder, 0);
    if (model->root == NULL) {
        free(model);
        return NULL;
    }

    /* 2) Keymap: prefer static table, else decode CBOR. */
    if (desc->keymap_static != NULL && desc->keymap_static_count > 0) {
        model->keymap_hashmap = buildKeymapFromStatic(desc->keymap_static,
                                                      desc->keymap_static_count);
    } else if (desc->keymap_cbor != NULL && desc->keymap_cbor_len > 0) {
        nanocbor_value_t keymap_decoder;
        nanocbor_decoder_init(&keymap_decoder, desc->keymap_cbor, desc->keymap_cbor_len);
        model->keymap_hashmap = cborToKeyMappingHashMap(&keymap_decoder);
    } else {
        model->keymap_hashmap = NULL;
    }
    if (model->keymap_hashmap == NULL) {
        freeCoreconf(model->root, true);
        free(model);
        return NULL;
    }

    /* 3) Clookup: prefer static table, else walk the instance tree. */
    if (desc->clookup_static != NULL && desc->clookup_static_count > 0) {
        model->clookup_hashmap = buildClookupFromStatic(desc->clookup_static,
                                                         desc->clookup_static_count);
    } else {
        model->clookup_hashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                            clookupHash, clookupCompare,
                                            NULL, NULL);
        if (model->clookup_hashmap == NULL) {
            hashmap_free(model->keymap_hashmap);
            freeCoreconf(model->root, true);
            free(model);
            return NULL;
        }
        buildCLookupHashmapFromCoreconf(model->root, model->clookup_hashmap, 0, 0);
    }

    return model;
}

CoreconfModelT *ccoreconfModelLoad(const uint8_t *instance_cbor,
                                     size_t instance_len,
                                     const uint8_t *keymap_cbor,
                                     size_t keymap_len) {
    CoreconfModelDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.instance_cbor = instance_cbor;
    desc.instance_cbor_len = instance_len;
    desc.keymap_cbor = keymap_cbor;
    desc.keymap_cbor_len = keymap_len;
    /* No static tables — desc fields stay NULL and the desc path falls
     * back to CBOR / tree-walking. */
    return ccoreconfModelLoadDesc(&desc);
}

void ccoreconfModelFree(CoreconfModelT *model) {
    if (model == NULL) {
        return;
    }
    if (model->clookup_hashmap != NULL) {
        /* hashmap_free will invoke the elfree callback (clookupFree for
         * the static-table path, NULL + manual free for the legacy path).
         * Do NOT also call freeCLookupHashmap() -- that's the legacy path
         * for NULL-callback hashmaps; calling it here would double-free. */
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