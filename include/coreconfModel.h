#ifndef CORECONF_MODEL_H
#define CORECONF_MODEL_H

#include <stddef.h>
#include <stdint.h>

#include "coreconfManipulation.h"
#include "coreconfTypes.h"
#include "hashmap.h"

/*
 * CoreconfModelT — a fully self-describing CORECONF model.
 *
 * One instance per YANG model.  A binary may have any number of these
 * registered at runtime; older single-model APIs continue to operate
 * on the hashmaps they always have, by calling the (model-aware)
 * functions in this header.
 *
 * Fields are public so that the generated <model>.c can initialise them
 * with static data tables from a CoreconfModelDesc.  Do not mutate the
 * fields after ccoreconfModelLoad returns.
 */
typedef struct CoreconfModel {
    /* Parsed instance data, decoded from the wire-format CBOR blob. */
    CoreconfValueT *root;

    /* YANG list key-mapping (sid -> [key-sid, ...]). */
    struct hashmap *keymap_hashmap;

    /* Reverse-clookup (sid -> [parent-sid, ...]) built from the instance tree. */
    struct hashmap *clookup_hashmap;

    /* Reserved for later steps in the multi-model refactor:
     *   struct hashmap *identifier_sid_hashmap;
     *   struct hashmap *sid_identifier_hashmap;
     *   struct hashmap *identifier_type_hashmap;
     *   struct hashmap *handler_hashmap;
     */
} CoreconfModelT;

/* ------------------------------------------------------------------------- *
 * Static model-data tables (declared by generated <model>.c files).         *
 * ------------------------------------------------------------------------- */

/* SID file item kind — mirrors pycoreconf's namespace() output. */
typedef enum {
    SID_NS_MODULE   = 0,
    SID_NS_IDENTITY = 1,
    SID_NS_DATA     = 2,
} SidNamespace;

/* One entry per item in the SID file. */
typedef struct {
    const char    *identifier;
    uint64_t       sid;
    SidNamespace   ns;
    int            is_list;
    const long    *key_sids;
    size_t         key_sids_count;
} SidEntryT;

/* One entry per YANG list in the key-mapping.  (Renamed from KeyMappingT
 * to avoid collision with the runtime hashmap-storage type in sid.h.) */
typedef struct {
    uint64_t       list_sid;
    const long    *key_sids;
    size_t         key_sids_count;
} SidKeyMappingT;

/* One entry per SID with at least one parent in the data tree. */
typedef struct {
    uint64_t       child_sid;
    const long    *parent_sids;
    size_t         parent_sids_count;
} CLookupEntryT;

/* Descriptor bundling every input ccoreconfModelLoadDesc can consume. */
typedef struct {
    const uint8_t *instance_cbor;
    size_t         instance_cbor_len;
    const uint8_t *keymap_cbor;
    size_t         keymap_cbor_len;

    /* Optional static tables.  Any NULL falls back to the CBOR path. */
    const SidEntryT   *sid_table;
    size_t            sid_count;
    const SidKeyMappingT *keymap_static;
    size_t               keymap_static_count;
    const CLookupEntryT *clookup_static;
    size_t               clookup_static_count;
} CoreconfModelDesc;

/* ------------------------------------------------------------------------- *
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

/**
 * Decode the wire-format CBOR blobs and assemble a CoreconfModelT.
 *
 * Convenience wrapper around ccoreconfModelLoadDesc that fills in a
 * CoreconfModelDesc from raw CBOR pointers (no static tables).
 *
 * @return A new CoreconfModelT, or NULL on decode / allocation failure.
 */
CoreconfModelT *ccoreconfModelLoad(const uint8_t *instance_cbor,
                                     size_t instance_len,
                                     const uint8_t *keymap_cbor,
                                     size_t keymap_len);

/**
 * Full-fat constructor.  Any of the static tables in `desc` may be NULL
 * (falling back to CBOR decoding or runtime tree-walking).
 *
 * @return A new CoreconfModelT, or NULL on decode / allocation failure.
 */
CoreconfModelT *ccoreconfModelLoadDesc(const CoreconfModelDesc *desc);

/**
 * Release a CoreconfModelT and all of its owned data.
 * Safe to call with NULL.
 */
void ccoreconfModelFree(CoreconfModelT *model);

/* ------------------------------------------------------------------------- *
 * Model-aware query helpers                                                 *
 *                                                                           *
 * Each helper has the same semantics as its <name> (no suffix)              *
 * counterpart in coreconfManipulation.h, but pulls state from the model    *
 * so the caller doesn't have to thread hashmaps through every call.         *
 * They are one-line wrappers around the legacy APIs — kept simple because   *
 * the legacy functions remain the canonical implementation.                 *
 * ------------------------------------------------------------------------- */

/** Walk the model's clookup and keymap to find the path required to
 *  reach `sid`.  Returns the same PathNodeT as the legacy
 *  findRequirementForSID, allocated on the heap. */
PathNodeT *findRequirementForSID_model(CoreconfModelT *model, uint64_t sid);

/** Navigate `model`'s tree along `headNode` using `requestKeys` and
 *  return the matching subtree.  Same semantics as the legacy
 *  examineCoreconfValue. */
CoreconfValueT *examineCoreconfValue_model(CoreconfModelT *model,
                                           DynamicLongListT *requestKeys,
                                           PathNodeT *headNode);

/** Build the clookup table (sid -> parent-sids) from the model's
 *  parsed tree.  Idempotent: any prior contents of
 *  model->clookup_hashmap are freed first. */
void buildCLookupHashmapFromCoreconf_model(CoreconfModelT *model);

#endif  // CORECONF_MODEL_H