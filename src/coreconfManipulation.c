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

/**
 * Functions to Create Path Node used for traversing the coreconf model
 */
PathNodeT *createPathNode(int64_t parentSID, DynamicLongListT *sidKeys) {
    PathNodeT *pathNode = malloc(sizeof(PathNodeT));
    pathNode->parentSID = parentSID;
    pathNode->sidKeys = sidKeys;
    pathNode->nextPathNode = NULL;
    return pathNode;
}

/**
 * Function to add a PathNode to the beginning of the PathNode linked list
 * * NOT returning the current address of the newPathNode
 */
PathNodeT *prependPathNode(PathNodeT *endNode, int64_t parentSID, DynamicLongListT *sidKeys) {
    PathNodeT *newPathNode = createPathNode(parentSID, sidKeys);
    newPathNode->nextPathNode = endNode;
    // endNode->nextPathNode = newPathNode;
    return newPathNode;
}

/*
 * Print the PathNode linked list
 */
void printPathNode(PathNodeT *pathNode) {
    int count = 0;
    PathNodeT *currentPathNode = pathNode;
    while (currentPathNode->parentSID != 0) {
        printf("parentSID = %" PRId64 " ", currentPathNode->parentSID);
        printDynamicLongList(currentPathNode->sidKeys);
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
        // freeDynamicLongList(currentPathNode->sidKeys);
        free(currentPathNode);
        currentPathNode = nextPathNode;
    }
}

/**
 * Function to find the requirement for a given SID
 */
PathNodeT *findRequirementForSID(uint64_t sid, struct hashmap *clookupHashmap, struct hashmap *keyMappingHashMap) {
    CLookupT *clookup = NULL;
    PathNodeT *pathNodes = createPathNode(0, NULL);
    pathNodes = prependPathNode(pathNodes, sid, NULL);

    int64_t currentSID = sid;
    while (currentSID != 0) {
        // Check if sid is in clookupHashmap
        clookup = (CLookupT *)hashmap_get(clookupHashmap, &(CLookupT){.childSID = currentSID});
        if (!clookup) {
            fprintf(stderr, "SID %" PRId64 " not found in the clookupHashmap\n", sid);
            return pathNodes;
        }

        // get the parent SID from clookup->dynamicLongList
        int64_t parentSID = popLong(clookup->dynamicLongList);

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
    CoreconfValueT *subTree = coreconfModel;
    int64_t previousSID = 0;

    // Iterate through the PathNode linked list
    PathNodeT *currentPathNode = headNode;
    while (currentPathNode->parentSID != 0) {
        // Get the parentSID from the currentPathNode
        int64_t parentSID = currentPathNode->parentSID;
        // Get the sidKeys from the currentPathNode
        DynamicLongListT *sidKeys = currentPathNode->sidKeys;

        // Switch to nextPathNode
        currentPathNode = currentPathNode->nextPathNode;

        int64_t deltaSID = parentSID - previousSID;
        // Fetch the subTree for deltaSID using getCoreconfHashMap
        subTree = getCoreconfHashMap(subTree->data.map_value, deltaSID);
        previousSID = parentSID;

        // Check if sidKeys is empty
        if (sidKeys == NULL || sidKeys->size == 0) {
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
            // Create sidKeyValueMatchDynamicLongList
            DynamicLongListT *sidKeyValueMatchDynamicLongList = malloc(sizeof(DynamicLongListT));
            initializeDynamicLongList(sidKeyValueMatchDynamicLongList);

            // Iterate through sidKeys
            for (int i = 0; i < (int)sidKeys->size; i++) {
                uint64_t sidKey = sidKeys->longList[i];

                uint64_t sidDiff = sidKey - parentSID;
                // Get value from element using sidDiff
                CoreconfValueT *elementValueCheck = getCoreconfHashMap(element->data.map_value, sidDiff);
                // Get the uint64_t value from elementValueCheck
                uint64_t elementValueCheckInteger = elementValueCheck->data.integer_value;

                // pop the value from requestKeysClone
                uint64_t keyValueCheck = (uint64_t)popLong(requestKeysClone);
                // If elementValueCheckLong == keyValueCheck then add sidKey to sidKeyValueMatchDynamicLongList
                if (elementValueCheckInteger == keyValueCheck)
                    addUniqueLong(sidKeyValueMatchDynamicLongList, (long)sidKey);
            }
            // Check if all the values in sidKey exist in sidKeyValueMatchDynamicLongList, if yes, then subTree =
            // element
            if (compareDynamicLongList(sidKeys, sidKeyValueMatchDynamicLongList)) {
                subTree = element;
                // NOTE Why are we not freeing requestKeys?
                // freeDynamicLongList(requestKeys);
                cloneDynamicLongList(requestKeysClone, requestKeys);
                break;
            }

            // Cleanup requestKeysClone
            freeDynamicLongList(requestKeysClone);
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
                uint64_t sidDiffValue = current->key;
                uint64_t childSIDValue = sidDiffValue + parentSID;

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
