#ifndef CORECONF_MANIPULATION_H
#define CORECONF_MANIPULATION_H

#include <stdio.h>
#include <stdlib.h>

#include "coreconfTypes.h"
#include "hashmap.h"
#include "sid.h"

typedef struct CLookup {
    int64_t childSID;
    DynamicLongListT *dynamicLongList;
} CLookupT;

typedef struct PathNode {
    int64_t parentSID;
    DynamicLongListT *SIDKeys;
    struct PathNode *nextPathNode;
} PathNodeT;

int clookupCompare(const void *a, const void *b, void *udata);
uint64_t clookupHash(const void *item, uint64_t seed0, uint64_t seed1);
void clookupFree(void *item);
void printCLookupHashmap(struct hashmap *clookupHashmap);

void buildCLookupHashmapFromCoreconf(CoreconfValueT *coreconfValue, struct hashmap *clookupHashmap, int64_t parentSID,
                                     int recursionDepth);

// Node related function headers
PathNodeT *createPathNode(int64_t parentSID, DynamicLongListT *SIDKeys);
PathNodeT *prependPathNode(PathNodeT *endNode, int64_t parentSID, DynamicLongListT *SIDKeys);
void printPathNode(PathNodeT *pathNode);
void freePathNode(PathNodeT *pathNode);
PathNodeT *findRequirementForSID(uint64_t SID, struct hashmap *clookupHashmap, struct hashmap *keyMappingHashMap);
/**
 * Walk `coreconfValue` along `headNode` (from findRequirementForSID), using
 * `requestKeys` to pick YANG list entries.  Keys are consumed from the END of
 * `requestKeys` (it is used as a stack).
 *
 * @return A new single-entry map { SID: node }, or NULL if not found.
 *         Ownership: the map itself is the caller's, but `node` is BORROWED
 *         from `coreconfValue`.  Free the result with
 *         freeExaminedCoreconfValue(), never with freeCoreconf(), and only
 *         while the model is still alive.
 */
CoreconfValueT *examineCoreconfValue(CoreconfValueT *coreconfValue, DynamicLongListT *requestKeys, PathNodeT *headNode);
/** Free a result of examineCoreconfValue: the wrapper map only, not the borrowed node. */
void freeExaminedCoreconfValue(CoreconfValueT *examined);

/**
 * Navigate to parent container of a target SID using delta encoding and PathNode
 * @param root Root of the coreconf model
 * @param pathNode Path to navigate (built by findRequirementForSID)
 * @param targetSID The final target SID
 * @param finalDeltaSID Output parameter for the final delta SID from parent to target
 * @return Pointer to parent container (hashmap), or NULL on error
 */
CoreconfValueT *navigateToParentContainer(CoreconfValueT *root, PathNodeT *pathNode, uint64_t targetSID,
                                          uint64_t *finalDeltaSID);

#endif  // CORECONF_MANIPULATION_H
