#include "../include/coreconfManipulation.h"

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/coreconfTypes.h"
#include "../include/hashmap.h"
#include "../include/serialization.h"

#define PATH_MAX_LENGTH 100
#define MAX_STACK_SIZE 100
#define SID_KEY_SIZE 21
#define MAX_CORECONF_RECURSION_DEPTH 50

/**
 * Functions related to CLookup HashMap
 */
int clookupCompare(const void *a, const void *b, void *udata) {
    // NOTE Keep udata unused for compatibility reasons
    (void)udata;

    const CLookupT *clookup1 = a;
    const CLookupT *clookup2 = b;

    return (clookup1->childSID != clookup2->childSID);
}

uint64_t clookupHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const CLookupT *clookup = (CLookupT *)item;
    return hashmap_murmur(&clookup->childSID, sizeof(uint64_t), seed0, seed1);
}

void clookupFree(void *item) {
    CLookupT *clookup = (CLookupT *)item;
    if (clookup && clookup->dynamicLongList) {
        freeDynamicLongList(clookup->dynamicLongList);
        clookup->dynamicLongList = NULL;
    }
}

/**
 * Functions to Create Path Node used for traversing the coreconf model
 */
PathNodeT *createPathNode(int64_t parentSID, DynamicLongListT *SIDKeys) {
    PathNodeT *pathNode = malloc(sizeof(PathNodeT));
    pathNode->parentSID = parentSID;
    // Make a deep copy to decouple PathNodeT from keyMappingHashMap ownership.
    // This prevents use-after-free when either structure is destroyed.
    if (SIDKeys) {
        DynamicLongListT *SIDKeysCopy = malloc(sizeof(DynamicLongListT));
        cloneDynamicLongList(SIDKeys, SIDKeysCopy);
        pathNode->SIDKeys = SIDKeysCopy;
    } else {
        pathNode->SIDKeys = NULL;
    }
    pathNode->nextPathNode = NULL;
    return pathNode;
}

/**
 * Function to add a PathNode to the beginning of the PathNode linked list
 * * NOT returning the current address of the newPathNode
 */
PathNodeT *prependPathNode(PathNodeT *endNode, int64_t parentSID, DynamicLongListT *SIDKeys) {
    PathNodeT *newPathNode = createPathNode(parentSID, SIDKeys);
    newPathNode->nextPathNode = endNode;
    // endNode->nextPathNode = newPathNode;
    return newPathNode;
}

/*
 * Print the PathNode linked list
 */
void printPathNode(PathNodeT *pathNode) {
    // NULL check
    if (pathNode == NULL) {
        printf("PathNode is NULL\n");
        return;
    }
    int count = 0;
    PathNodeT *currentPathNode = pathNode;
    while (currentPathNode->parentSID != 0) {
        printf("parentSID = %" PRId64 " ", currentPathNode->parentSID);
        printDynamicLongList(currentPathNode->SIDKeys);
        printf("\n");
        currentPathNode = currentPathNode->nextPathNode;
        count++;
    }

    if (count == 0)
        printf("parentSID = %" PRId64 " thus the given node is a parent node\n", currentPathNode->parentSID);
}

/*
Function to safely free the PathNode linked list
*/
void freePathNode(PathNodeT *headNode) {
    PathNodeT *currentPathNode = headNode;
    PathNodeT *nextPathNode = NULL;
    while (currentPathNode != NULL) {
        nextPathNode = currentPathNode->nextPathNode;
        if (currentPathNode->SIDKeys != NULL) {
            freeDynamicLongList(currentPathNode->SIDKeys);
        }
        free(currentPathNode);
        currentPathNode = nextPathNode;
    }
}

/**
 * Function to find the requirement for a given SID
 */
PathNodeT *findRequirementForSID(uint64_t SID, struct hashmap *clookupHashmap, struct hashmap *keyMappingHashMap) {
    CLookupT *clookup = NULL;
    PathNodeT *pathNodes = createPathNode(0, NULL);

    // Check if a keyMapping object exists for the given SID
    const KeyMappingT *keyMappingForGivenSID = hashmap_get(keyMappingHashMap, &(KeyMappingT){.key = SID});
    if (keyMappingForGivenSID) {
        // Create a new PathNode with the given SID and the keyMappingForGivenSID->dynamicLongList
        pathNodes = prependPathNode(pathNodes, SID, keyMappingForGivenSID->dynamicLongList);
    } else {
        pathNodes = prependPathNode(pathNodes, SID, NULL);
    }

    int64_t currentSID = SID;
    while (currentSID != 0) {
        // Check if SID is in clookupHashmap
        clookup = (CLookupT *)hashmap_get(clookupHashmap, &(CLookupT){.childSID = currentSID});
        if (!clookup) {
            fprintf(stderr, "SID %" PRId64 " not found in the clookupHashmap\n", SID);
            return NULL;
        }

        // get the parent SID from clookup->dynamicLongList
        int64_t parentSID = peekLong(clookup->dynamicLongList);

        // if parentSID is 0 then break
        if (parentSID != 0) {
            // get assosciated keys from parentSID from keyMappingHashMap
            const KeyMappingT *keyMapping = hashmap_get(keyMappingHashMap, &(KeyMappingT){.key = parentSID});
            // No keyMapping found then add a blank
            if (keyMapping) {
                // NOTE Don't forget to create an pathNode and pass that to this function
                // prepend a new PathNode with parentSID and keyMapping->dynamicLongList
                pathNodes = prependPathNode(pathNodes, parentSID, keyMapping->dynamicLongList);
            } else {
                // No keys for this currentSID, so add a blank dynamicLongList;
                pathNodes = prependPathNode(pathNodes, parentSID, NULL);
            }
        }

        currentSID = parentSID;
    }

    // Add a blank node with currentSID as 0 and blank dynamicLongList so it prints well
    return pathNodes;
}

// Examine CORECONF by traversing through headNode
CoreconfValueT *examineCoreconfValue(CoreconfValueT *coreconfModel, DynamicLongListT *requestKeys,
                                     PathNodeT *headNode) {
    // NULL Checks
    if (coreconfModel == NULL) {
        fprintf(stderr, "coreconfModel is NULL\n");
        return NULL;
    }

    if (headNode == NULL) {
        fprintf(stderr, "headNode is NULL\n");
        return NULL;
    }

    CoreconfValueT *subTree = coreconfModel;
    int64_t previousSID = 0;

    // Iterate through the PathNode linked list
    PathNodeT *currentPathNode = headNode;
    while (currentPathNode->parentSID != 0) {
        // Get the parentSID from the currentPathNode
        int64_t parentSID = currentPathNode->parentSID;
        // Get the SIDKeys from the currentPathNode
        DynamicLongListT *SIDKeys = currentPathNode->SIDKeys;

        // Switch to nextPathNode
        currentPathNode = currentPathNode->nextPathNode;

        int64_t deltaSID = parentSID - previousSID;
        // Fetch the subTree for deltaSID using getCoreconfHashMap
        subTree = getCoreconfHashMap(subTree->data.map_value, deltaSID);

        // If we try to access a list key that is not yet populated, then this will return NULL
        // But if that happens it is not an error, so we need to return gracefully
        if (subTree == NULL) {
            return NULL;
        }

        previousSID = parentSID;

        // Check if SIDKeys is empty
        if (SIDKeys == NULL || SIDKeys->size == 0) {
            continue;
        }

        // Check if subTree is not a CoreconfArray
        if (subTree->type != CORECONF_ARRAY) {
            fprintf(stderr, "subTree is not a CoreconfArray\n");
            return NULL;
        }

        // Iterate through the subTree
        size_t arraySize = subTree->data.array_value->size;
        for (size_t i = 0; i < arraySize; i++) {
            CoreconfValueT *element = &subTree->data.array_value->elements[i];

            // Create a new DynamicLongListT
            DynamicLongListT *requestKeysClone = malloc(sizeof(DynamicLongListT));
            // Clone requestKeys
            cloneDynamicLongList(requestKeys, requestKeysClone);
            // Create SIDKeyValueMatchDynamicLongList
            DynamicLongListT *SIDKeyValueMatchDynamicLongList = malloc(sizeof(DynamicLongListT));
            initializeDynamicLongList(SIDKeyValueMatchDynamicLongList);

            // Iterate through SIDKeys
            for (int i = 0; i < (int)SIDKeys->size; i++) {
                uint64_t SIDKey = SIDKeys->longList[i];

                uint64_t SIDDiff = SIDKey - parentSID;
                // Get value from element using SIDDiff
                CoreconfValueT *elementValueCheck = getCoreconfHashMap(element->data.map_value, SIDDiff);
                // Get the uint64_t value from elementValueCheck
                uint64_t elementValueCheckInteger = getCoreconfValueAsUint64(elementValueCheck);

                // pop the value from requestKeysClone
                uint64_t keyValueCheck = (uint64_t)popLong(requestKeysClone);
                // If elementValueCheckLong == keyValueCheck then add SIDKey to SIDKeyValueMatchDynamicLongList
                if (elementValueCheckInteger == keyValueCheck)
                    addUniqueLong(SIDKeyValueMatchDynamicLongList, (long)SIDKey);
            }
            // Check if all the values in SIDKey exist in SIDKeyValueMatchDynamicLongList, if yes, then subTree =
            // element
            if (compareDynamicLongList(SIDKeys, SIDKeyValueMatchDynamicLongList)) {
                subTree = element;
                cloneDynamicLongList(requestKeysClone, requestKeys);
                freeDynamicLongList(requestKeysClone);
                freeDynamicLongList(SIDKeyValueMatchDynamicLongList);
                break;
            }

            freeDynamicLongList(requestKeysClone);
            freeDynamicLongList(SIDKeyValueMatchDynamicLongList);
        }
    }

    CoreconfValueT *returnMap = createCoreconfHashmap();
    // Insert key-value:  previousSID (which is the parentSID) and Subtree Value
    insertCoreconfHashMap(returnMap->data.map_value, previousSID, subTree);
    return returnMap;
}

void buildCLookupHashmapFromCoreconf(CoreconfValueT *coreconfValue, struct hashmap *clookupHashmap, int64_t parentSID,
                                     int recursionDepth) {
    // If the depth exceeds than the MAX then return
    if (recursionDepth > MAX_CORECONF_RECURSION_DEPTH) return;

    // Check if the type is a CORECONF Hashmap
    if (coreconfValue->type == CORECONF_HASHMAP) {
        for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
            CoreconfObjectT *current = coreconfValue->data.map_value->table[i];
            while (current != NULL) {
                uint64_t SIDDiffValue = current->key;
                uint64_t childSIDValue = SIDDiffValue + parentSID;

                // Get the dynamicLongList for the childSIDValue from clookupHashmap
                // if there is none, make a new dynamicLongList element and add it to clookupHashmap
                CLookupT *clookup = (CLookupT *)hashmap_get(clookupHashmap, &(CLookupT){.childSID = childSIDValue});
                if (!clookup) {
                    clookup = malloc(sizeof(CLookupT));
                    clookup->childSID = childSIDValue;
                    clookup->dynamicLongList = malloc(sizeof(DynamicLongListT));
                    initializeDynamicLongList(clookup->dynamicLongList);
                    // Add the parentSID only if it doesn't exist in the dynamicLongList
                    addUniqueLong(clookup->dynamicLongList, parentSID);
                    hashmap_set(clookupHashmap, clookup);
                } else {
                    // Add parentSID to the dynamicLongList only if it doesn't exist already
                    addUniqueLong(clookup->dynamicLongList, parentSID);
                }
                // Recursively call buildCLookupHashmapFromCoreconf for the value
                buildCLookupHashmapFromCoreconf(current->value, clookupHashmap, childSIDValue, recursionDepth + 1);
                current = current->next;
            }
        }
    } else if (coreconfValue->type == CORECONF_ARRAY) {
        for (size_t i = 0; i < coreconfValue->data.array_value->size; i++) {
            CoreconfValueT *arrayElement = &coreconfValue->data.array_value->elements[i];
            buildCLookupHashmapFromCoreconf(arrayElement, clookupHashmap, parentSID, recursionDepth + 1);
        }
    } else {
        // LEAVES
        // Do nothing
    }
}

/**
 * Function to iterate through cLookupHashmap and print its contents
 */
void printCLookupHashmap(struct hashmap *clookupHashmap) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(clookupHashmap, &iter, &item)) {
        CLookupT *clookupObject = item;
        printf("(Child SID =%lu) ", (long)clookupObject->childSID);
        printDynamicLongList(clookupObject->dynamicLongList);
    }
}

// Assume long value is 64bit long,
void long2str(char *stringValue, long longValue) { sprintf(stringValue, "%ld", longValue); }

void freeCLookupHashmap(struct hashmap *map) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(map, &iter, &item)) {
        CLookupT *clookup = (CLookupT *)item;
        if (clookup->dynamicLongList) {
            freeDynamicLongList(clookup->dynamicLongList);
            // NULL it, like clookupFree, so a later free of the same map is harmless
            clookup->dynamicLongList = NULL;
        }
    }
}

/**
 * Navigate to parent container of a target SID using delta encoding and PathNode
 * This function traverses the coreconf model hierarchy using the PathNode chain,
 * applying delta SID encoding at each level to find the parent container.
 *
 * @param root Root of the coreconf model (must be a hashmap)
 * @param pathNode Path to navigate (built by findRequirementForSID)
 * @param targetSID The final target SID
 * @param finalDeltaSID Output parameter for the final delta SID from parent to target
 * @return Pointer to parent container (hashmap), or NULL on error
 */
CoreconfValueT *navigateToParentContainer(CoreconfValueT *root, PathNodeT *pathNode, uint64_t targetSID,
                                          uint64_t *finalDeltaSID) {
    if (root == NULL || pathNode == NULL || finalDeltaSID == NULL) {
        printf("Error: NULL parameter in navigateToParentContainer\n");
        return NULL;
    }

    CoreconfValueT *current = root;
    PathNodeT *path = pathNode;
    uint64_t previousSID = 0;

    while (path->parentSID != (int64_t)targetSID) {
        // Get the parentSID from the currentPathNode
        int64_t parentSID = path->parentSID;

        // Switch to nextPathNode
        path = path->nextPathNode;

        int64_t deltaSID = parentSID - previousSID;
        // Fetch the subTree for deltaSID using getCoreconfHashMap
        current = getCoreconfHashMap(current->data.map_value, deltaSID);
        previousSID = parentSID;
    }

    // Calculate final delta SID from parent to target
    *finalDeltaSID = targetSID - previousSID;

    // Verify parent is a hashmap
    if (current->type != CORECONF_HASHMAP && current->type != CORECONF_ARRAY) {
        printf("Error: Parent container is not a hashmap or array for SID %lu (got type %d)\n", targetSID,
               current->type);
        return NULL;
    }

    return current;
}
