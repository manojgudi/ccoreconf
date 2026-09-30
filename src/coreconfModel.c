/*
 * src/coreconfModel.c — CoreconfModelT lifecycle and model-aware
 * wrapper functions.
 *
 * The functions in this file are one-line passthrough wrappers around
 * the legacy APIs in coreconfManipulation.c.  Their only purpose is to
 * give multi-model code a single handle to pass around instead of
 * (root, clookupHashmap, keymapHashmap) tuples.
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

/* ---- internal: build hashmap from a static SIDKeyMappingT[] table ---- */
static struct hashmap *buildKeymapFromStatic(const SIDKeyMappingT *tbl,
                                              size_t count) {
    struct hashmap *km = hashmap_new(sizeof(KeyMappingT), 0, 0, 0,
                                      keyMappingHash, keyMappingCompare,
                                      keyMappingFree, NULL);
    if (km == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        KeyMappingT entry;
        entry.key = (int64_t)tbl[i].listSID;
        entry.dynamicLongList = createDynamicLongList();
        if (entry.dynamicLongList == NULL) {
            hashmap_free(km);
            return NULL;
        }
        for (size_t k = 0; k < tbl[i].keySIDsCount; k++) {
            addLong(entry.dynamicLongList, tbl[i].keySIDs[k]);
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
        entry.childSID = (int64_t)tbl[i].childSID;
        entry.dynamicLongList = createDynamicLongList();
        if (entry.dynamicLongList == NULL) {
            hashmap_free(cm);
            return NULL;
        }
        for (size_t p = 0; p < tbl[i].parentSIDsCount; p++) {
            addUniqueLong(entry.dynamicLongList, tbl[i].parentSIDs[p]);
        }
        hashmap_set(cm, &entry);
    }
    return cm;
}

/* ---- internal: build identifier<->SID hashmaps from SIDEntryT[] ---- */

/* Layout for an identifierSIDHashmap entry.  Key is the identifier
 * string, value is the SID.  The string pointer is borrowed from the
 * SIDEntryT[] and is valid for the model's lifetime. */
typedef struct {
    const char *identifier;
    uint64_t    SID;
} IdentSIDValueT;

/* Layout for a SIDIdentifierHashmap entry.  Key is the SID, value
 * is the borrowed identifier string. */
typedef struct {
    uint64_t    SID;
    const char *identifier;
} SIDIdentValueT;

static uint64_t identSIDHash(const void *item, uint64_t s0, uint64_t s1) {
    const IdentSIDValueT *e = (const IdentSIDValueT *)item;
    return hashmap_sip(e->identifier, strlen(e->identifier), s0, s1);
}

static int identSIDCompare(const void *a, const void *b, void *udata) {
    (void)udata;
    const IdentSIDValueT *ea = (const IdentSIDValueT *)a;
    const IdentSIDValueT *eb = (const IdentSIDValueT *)b;
    return strcmp(ea->identifier, eb->identifier) != 0;
}

static uint64_t SIDIdentHash(const void *item, uint64_t s0, uint64_t s1) {
    const SIDIdentValueT *e = (const SIDIdentValueT *)item;
    return hashmap_murmur(&e->SID, sizeof(e->SID), s0, s1);
}

static int SIDIdentCompare(const void *a, const void *b, void *udata) {
    (void)udata;
    const SIDIdentValueT *ea = (const SIDIdentValueT *)a;
    const SIDIdentValueT *eb = (const SIDIdentValueT *)b;
    return ea->SID != eb->SID;
}

static struct hashmap *buildIdentifierSIDHashmap(const SIDEntryT *tbl,
                                                 size_t count) {
    struct hashmap *m = hashmap_new(sizeof(IdentSIDValueT), 0, 0, 0,
                                     identSIDHash, identSIDCompare,
                                     NULL, NULL);
    if (m == NULL) return NULL;
    for (size_t i = 0; i < count; i++) {
        if (tbl[i].identifier == NULL || tbl[i].identifier[0] == '\0') continue;
        IdentSIDValueT e;
        e.identifier = tbl[i].identifier;
        e.SID        = tbl[i].SID;
        hashmap_set(m, &e);
    }
    return m;
}

static struct hashmap *buildSIDIdentifierHashmap(const SIDEntryT *tbl,
                                                 size_t count) {
    struct hashmap *m = hashmap_new(sizeof(SIDIdentValueT), 0, 0, 0,
                                     SIDIdentHash, SIDIdentCompare,
                                     NULL, NULL);
    if (m == NULL) return NULL;
    for (size_t i = 0; i < count; i++) {
        if (tbl[i].identifier == NULL || tbl[i].identifier[0] == '\0') continue;
        SIDIdentValueT e;
        e.SID        = tbl[i].SID;
        e.identifier = tbl[i].identifier;
        hashmap_set(m, &e);
    }
    return m;
}

/* ------------------------------------------------------------------------- *
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

CoreconfModelT *ccoreconfModelLoadDesc(const CoreconfModelDescT *desc) {
    if (desc == NULL || desc->instanceCBOR == NULL) {
        return NULL;
    }

    CoreconfModelT *model = (CoreconfModelT *)calloc(1, sizeof(CoreconfModelT));
    if (model == NULL) {
        return NULL;
    }

    /* 1) Decode the instance tree (always required — it's the wire payload). */
    nanocbor_value_t instanceDecoder;
    nanocbor_decoder_init(&instanceDecoder, desc->instanceCBOR, desc->instanceCBORLen);
    model->root = cborToCoreconfValue(&instanceDecoder, 0);
    if (model->root == NULL) {
        free(model);
        return NULL;
    }

    /* 2) Keymap: prefer static table, else decode CBOR. */
    if (desc->keymapStatic != NULL && desc->keymapStaticCount > 0) {
        model->keymapHashmap = buildKeymapFromStatic(desc->keymapStatic,
                                                      desc->keymapStaticCount);
    } else if (desc->keymapCBOR != NULL && desc->keymapCBORLen > 0) {
        nanocbor_value_t keymapDecoder;
        nanocbor_decoder_init(&keymapDecoder, desc->keymapCBOR, desc->keymapCBORLen);
        model->keymapHashmap = cborToKeyMappingHashMap(&keymapDecoder);
    } else {
        model->keymapHashmap = NULL;
    }
    if (model->keymapHashmap == NULL) {
        freeCoreconf(model->root, true);
        free(model);
        return NULL;
    }

    /* 3) Clookup: prefer static table, else walk the instance tree. */
    if (desc->clookupStatic != NULL && desc->clookupStaticCount > 0) {
        model->clookupHashmap = buildClookupFromStatic(desc->clookupStatic,
                                                         desc->clookupStaticCount);
    } else {
        model->clookupHashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                            clookupHash, clookupCompare,
                                            NULL, NULL);
        if (model->clookupHashmap == NULL) {
            hashmap_free(model->keymapHashmap);
            freeCoreconf(model->root, true);
            free(model);
            return NULL;
        }
        buildCLookupHashmapFromCoreconf(model->root, model->clookupHashmap, 0, 0);
    }

    /* 4) identifier<->SID hashmaps, populated from the static SID table. */
    if (desc->SIDTable != NULL && desc->SIDCount > 0) {
        model->identifierSIDHashmap = buildIdentifierSIDHashmap(
            desc->SIDTable, desc->SIDCount);
        model->SIDIdentifierHashmap = buildSIDIdentifierHashmap(
            desc->SIDTable, desc->SIDCount);
        /* Don't fail the whole load if these are NULL -- the rest of
         * the model is still usable. */
    }

    return model;
}

CoreconfModelT *ccoreconfModelLoad(const uint8_t *instanceCBOR,
                                   size_t instanceLen,
                                   const uint8_t *keymapCBOR,
                                   size_t keymapLen) {
    CoreconfModelDescT desc;
    memset(&desc, 0, sizeof(desc));
    desc.instanceCBOR = instanceCBOR;
    desc.instanceCBORLen = instanceLen;
    desc.keymapCBOR = keymapCBOR;
    desc.keymapCBORLen = keymapLen;
    /* No static tables — desc fields stay NULL and the desc path falls
     * back to CBOR / tree-walking. */
    return ccoreconfModelLoadDesc(&desc);
}

void ccoreconfModelFree(CoreconfModelT *model) {
    if (model == NULL) {
        return;
    }
    if (model->clookupHashmap != NULL) {
        /* hashmap_free will invoke the elfree callback (clookupFree for
         * the static-table path, NULL + manual free for the legacy path).
         * Do NOT also call freeCLookupHashmap() -- that's the legacy path
         * for NULL-callback hashmaps; calling it here would double-free. */
        hashmap_free(model->clookupHashmap);
        model->clookupHashmap = NULL;
    }
    if (model->keymapHashmap != NULL) {
        hashmap_free(model->keymapHashmap);
        model->keymapHashmap = NULL;
    }
    if (model->identifierSIDHashmap != NULL) {
        hashmap_free(model->identifierSIDHashmap);
        model->identifierSIDHashmap = NULL;
    }
    if (model->SIDIdentifierHashmap != NULL) {
        hashmap_free(model->SIDIdentifierHashmap);
        model->SIDIdentifierHashmap = NULL;
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

PathNodeT *ccoreconfModelFindRequirementForSID(CoreconfModelT *model, uint64_t SID) {
    if (model == NULL || model->clookupHashmap == NULL || model->keymapHashmap == NULL) {
        return NULL;
    }
    return findRequirementForSID(SID, model->clookupHashmap, model->keymapHashmap);
}

CoreconfValueT *ccoreconfModelExamineCoreconfValue(CoreconfModelT *model,
                                                   DynamicLongListT *requestKeys,
                                                   PathNodeT *headNode) {
    if (model == NULL || model->root == NULL) {
        return NULL;
    }
    return examineCoreconfValue(model->root, requestKeys, headNode);
}

void ccoreconfModelBuildCLookupHashmap(CoreconfModelT *model) {
    if (model == NULL || model->root == NULL) {
        return;
    }
    /* Free any pre-existing clookup entries before rebuilding. */
    if (model->clookupHashmap != NULL) {
        freeCLookupHashmap(model->clookupHashmap);
        hashmap_clear(model->clookupHashmap, false);
    } else {
        model->clookupHashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0,
                                            clookupHash, clookupCompare, NULL, NULL);
        if (model->clookupHashmap == NULL) {
            return;
        }
    }
    buildCLookupHashmapFromCoreconf(model->root, model->clookupHashmap, 0, 0);
}

/* ------------------------------------------------------------------------- *
 * Step 5: identifier<->SID lookup helpers                                *
 * ------------------------------------------------------------------------- */

uint64_t ccoreconfModelLookupSID(CoreconfModelT *model, const char *identifier) {
    if (model == NULL || identifier == NULL ||
        model->identifierSIDHashmap == NULL) {
        return 0;
    }
    IdentSIDValueT key;
    key.identifier = identifier;
    const IdentSIDValueT *found =
        (const IdentSIDValueT *)hashmap_get(model->identifierSIDHashmap, &key);
    return (found != NULL) ? found->SID : 0;
}

const char *ccoreconfModelLookupIdentifier(CoreconfModelT *model, uint64_t SID) {
    if (model == NULL || model->SIDIdentifierHashmap == NULL) {
        return NULL;
    }
    SIDIdentValueT key;
    key.SID = SID;
    const SIDIdentValueT *found =
        (const SIDIdentValueT *)hashmap_get(model->SIDIdentifierHashmap, &key);
    return (found != NULL) ? found->identifier : NULL;
}