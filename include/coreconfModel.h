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
 * fields after ccoreconf_model_load returns.
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
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

/**
 * Decode the wire-format CBOR blobs and assemble a CoreconfModelT.
 *
 * @param instance_cbor  CORECONF instance bytes (the same bytes that the
 *                       wire protocol carries, typically produced by
 *                       tools/prepareModel.py cbor).
 * @param instance_len   Length of instance_cbor in bytes.
 * @param keymap_cbor    KeyMapping CBOR bytes (the same format that the
 *                       legacy coreconf_model_cbor.h used to embed).
 * @param keymap_len     Length of keymap_cbor in bytes.
 *
 * @return A new CoreconfModelT, or NULL on decode / allocation failure.
 *         Free with ccoreconf_model_free when done.
 *
 * @note   Older call sites that build the model by hand
 *         (nanocbor_decoder_init -> cborToCoreconfValue -> ... ->
 *         buildCLookupHashmapFromCoreconf) continue to work without
 *         changes — they just don't get the convenience of the model
 *         wrapper.
 */
CoreconfModelT *ccoreconf_model_load(const uint8_t *instance_cbor,
                                     size_t instance_len,
                                     const uint8_t *keymap_cbor,
                                     size_t keymap_len);

/**
 * Release a CoreconfModelT and all of its owned data.
 * Safe to call with NULL.
 */
void ccoreconf_model_free(CoreconfModelT *model);

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