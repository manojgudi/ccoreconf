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
 * with static data tables from a CoreconfModelDescT.  Do not mutate the
 * fields after ccoreconfModelLoad returns.
 */
typedef struct CoreconfModel {
    /* Parsed instance data, decoded from the wire-format CBOR blob. */
    CoreconfValueT *root;

    /* YANG list key-mapping (SID -> [key SID, ...]). */
    struct hashmap *keymapHashmap;

    /* Reverse-clookup (SID -> [parent SID, ...]) built from the instance tree. */
    struct hashmap *clookupHashmap;

    /* Step 5: bidirectional SID <-> identifier hashmaps, built from the
     * SIDEntryT[] static descriptor at load time when one is provided.
     * identifierSIDHashmap stores strings owned by the SIDEntryT
     * (not copied); entries survive as long as the model's descriptor. */
    struct hashmap *identifierSIDHashmap;   /* const char *identifier -> uint64_t SID */
    struct hashmap *SIDIdentifierHashmap;   /* uint64_t SID -> const char *identifier */

    /* Reserved for step 6:
     *   struct hashmap *identifierTypeHashmap;
     *   struct hashmap *handlerHashmap;
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
} SIDNamespace;

/* One entry per item in the SID file. */
typedef struct {
    const char    *identifier;
    uint64_t       SID;
    SIDNamespace   ns;
    int            isList;
    const long    *keySIDs;
    size_t         keySIDsCount;
} SIDEntryT;

/* One entry per YANG list in the key-mapping.  (Renamed from KeyMappingT
 * to avoid collision with the runtime hashmap-storage type in sid.h.) */
typedef struct {
    uint64_t       listSID;
    const long    *keySIDs;
    size_t         keySIDsCount;
} SIDKeyMappingT;

/* One entry per SID with at least one parent in the data tree. */
typedef struct {
    uint64_t       childSID;
    const long    *parentSIDs;
    size_t         parentSIDsCount;
} CLookupEntryT;

/* Descriptor bundling every input ccoreconfModelLoadDesc can consume. */
typedef struct {
    const uint8_t *instanceCBOR;
    size_t         instanceCBORLen;
    const uint8_t *keymapCBOR;
    size_t         keymapCBORLen;

    /* Optional static tables.  Any NULL falls back to the CBOR path. */
    const SIDEntryT   *SIDTable;
    size_t            SIDCount;
    const SIDKeyMappingT *keymapStatic;
    size_t               keymapStaticCount;
    const CLookupEntryT *clookupStatic;
    size_t               clookupStaticCount;
} CoreconfModelDescT;

/* ------------------------------------------------------------------------- *
 * Lifecycle                                                                 *
 * ------------------------------------------------------------------------- */

/**
 * Decode the wire-format CBOR blobs and assemble a CoreconfModelT.
 *
 * Convenience wrapper around ccoreconfModelLoadDesc that fills in a
 * CoreconfModelDescT from raw CBOR pointers (no static tables).
 *
 * @return A new CoreconfModelT, or NULL on decode / allocation failure.
 */
CoreconfModelT *ccoreconfModelLoad(const uint8_t *instanceCBOR,
                                   size_t instanceLen,
                                   const uint8_t *keymapCBOR,
                                   size_t keymapLen);

/**
 * Full-fat constructor.  Any of the static tables in `desc` may be NULL
 * (falling back to CBOR decoding or runtime tree-walking).
 *
 * @return A new CoreconfModelT, or NULL on decode / allocation failure.
 */
CoreconfModelT *ccoreconfModelLoadDesc(const CoreconfModelDescT *desc);

/**
 * Release a CoreconfModelT and all of its owned data.
 * Safe to call with NULL.
 */
void ccoreconfModelFree(CoreconfModelT *model);

/* ------------------------------------------------------------------------- *
 * Step 5: identifier <-> SID lookup helpers                               *
 *                                                                           *
 * Both functions are no-ops (return 0 / NULL) when the model was loaded    *
 * without a SIDEntryT[] static table.  Otherwise they consult the          *
 * identifierSIDHashmap / SIDIdentifierHashmap populated at load time.  *
 * ------------------------------------------------------------------------- */

/**
 * Look up the SID for a YANG identifier (e.g. "/sensor:sensor/healthValue").
 *
 * @return The numeric SID (>0), or 0 if not found / model has no static
 *         SID table / args are NULL.  The returned identifier string is
 *         owned by the model's SIDEntryT[] table; do not free.
 */
uint64_t ccoreconfModelLookupSID(CoreconfModelT *model, const char *identifier);

/**
 * Reverse lookup: identifier string for a numeric SID.
 *
 * @return A borrowed pointer to the identifier string, or NULL if not
 *         found.  The returned pointer is owned by the model's static
 *         SIDEntryT[] table and is valid for the lifetime of the model.
 */
const char *ccoreconfModelLookupIdentifier(CoreconfModelT *model, uint64_t SID);

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
 *  reach `SID`.  Returns the same PathNodeT as the legacy
 *  findRequirementForSID, allocated on the heap. */
PathNodeT *ccoreconfModelFindRequirementForSID(CoreconfModelT *model, uint64_t SID);

/** Navigate `model`'s tree along `headNode` using `requestKeys` and
 *  return the matching subtree.  Same semantics as the legacy
 *  examineCoreconfValue. */
CoreconfValueT *ccoreconfModelExamineCoreconfValue(CoreconfModelT *model,
                                                   DynamicLongListT *requestKeys,
                                                   PathNodeT *headNode);

/** Build the clookup table (SID -> parent SIDs) from the model's
 *  parsed tree.  Idempotent: any prior contents of
 *  model->clookupHashmap are freed first. */
void ccoreconfModelBuildCLookupHashmap(CoreconfModelT *model);

#endif  // CORECONF_MODEL_H