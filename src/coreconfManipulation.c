#include "../include/coreconfManipulation.h"

#include <inttypes.h>
#include <jansson.h>
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

/*
Examine CORECONF by traversing through headNode
*/
json_t *examineCoreconf(json_t *coreconfModel, DynamicLongListT *requestKeys, PathNodeT *headNode) {
    json_t *subTree = coreconfModel;
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
        // Fetch the subTree for deltaSID, dont forget to convert deltaSID into string
        char deltaSIDString[SID_KEY_SIZE];
        snprintf(deltaSIDString, SID_KEY_SIZE, "%" PRId64, deltaSID);
        subTree = json_object_get(subTree, deltaSIDString);
        previousSID = parentSID;

        // Check if sidKeys is empty
        if (sidKeys == NULL || sidKeys->size == 0) {
            continue;
        }

        // Check if subTree is not a json list
        if (!json_is_array(subTree)) {
            fprintf(stderr, "subTree is not a json list\n");
            return NULL;
        }

        // Iterate through the subTree using json list methods to iterate
        size_t arrayLength = json_array_size(subTree);
        for (size_t i = 0; i < arrayLength; i++) {
            json_t *element = json_array_get(subTree, i);
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
                char sidDiffString[SID_KEY_SIZE];
                snprintf(sidDiffString, SID_KEY_SIZE, "%" PRId64, sidDiff);
                // Get value from element using sidDiffString
                json_t *elementValueCheck = json_object_get(element, sidDiffString);
                // Get the uint64_t value from elementValueCheck
                uint64_t elementValueCheckLong = (uint64_t)json_integer_value(elementValueCheck);

                // Pop the value from requestKeysClone
                uint64_t keyValueCheck = (uint64_t)popLong(requestKeysClone);

                // If elementValueCheckLong == keyValueCheck then add sidKey to sidKeyValueMatchDynamicLongList
                if (elementValueCheckLong == keyValueCheck) {
                    addUniqueLong(sidKeyValueMatchDynamicLongList, (long)sidKey);
                }
                // Should there be an else part?
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

    json_t *returnMap = json_object();
    // Convert parentSID to a string
    char parentSIDString[SID_KEY_SIZE];
    snprintf(parentSIDString, SID_KEY_SIZE, "%" PRId64, previousSID);
    json_object_set(returnMap, parentSIDString, subTree);
    return returnMap;
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

/**
 * Build a Chump Lookup or a CLookup hashmap from the coreconf model in JSON
 */
void buildCLookupHashmap(json_t *coreconfModel, struct hashmap *clookupHashmap, int64_t parentSID, int recursionDepth) {
    // If the depth exceeds than the MAX then return
    if (recursionDepth > MAX_CORECONF_RECURSION_DEPTH) return;

    if (json_is_object(coreconfModel)) {
        const char *key;
        json_t *value;
        // Iterate through coreconfModel and build the CLookup hashmap

        json_object_foreach(coreconfModel, key, value) {
            uint64_t sidDiffValue = char2int64((char *)key);
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
            // Recursively call buildCLookupHashmap for the value
            buildCLookupHashmap(value, clookupHashmap, childSIDValue, recursionDepth + 1);
        }
    } else if (json_is_array(coreconfModel)) {
        size_t arrayLength = json_array_size(coreconfModel);
        for (int i = 0; i < (int)arrayLength; i++) {
            json_t *arrayElement = json_array_get(coreconfModel, i);
            buildCLookupHashmap(arrayElement, clookupHashmap, parentSID, recursionDepth + 1);
        }
    } else {
        // LEAVES
        // Do nothing
    }
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
        printf("(Child SID =%lu) ", clookupObject->childSID);
        printDynamicLongList(clookupObject->dynamicLongList);
    }
}

/**
 * Stack related methods
 */

// Function to initialize the stack
void initStackStorage(StackStorageT *stackStorage, int capacity) {
    stackStorage->stackElements = (StackElementT **)malloc(capacity * sizeof(StackElementT *));
    stackStorage->top = -1;
    stackStorage->capacity = capacity;
    stackStorage->size = 0;
}
// Function to check if the stack is empty
bool isEmpty(StackStorageT *stackStorage) { return (stackStorage->top == -1) || (stackStorage->size == 0); }

// Function to check if the stack is full
bool isFull(StackStorageT *stackStorage) { return stackStorage->top == stackStorage->capacity - 1; }

// Function to push an element onto the stack
void push(StackStorageT *stackStorage, StackElementT *stackElement) {
    if (isFull(stackStorage))
        fprintf(stderr, "Internal Stack overflow! Cannot push element");
    else {
        stackStorage->stackElements[++stackStorage->top] = stackElement;
        stackStorage->size++;
    }
}

// Function to pop an element from the stack
StackElementT *pop(StackStorageT *stackStorage) {
    if (isEmpty(stackStorage)) {
        fprintf(stderr, "Internal Stack Underflow! Cannot pop element");
        return NULL;
    } else {
        stackStorage->size--;
        return stackStorage->stackElements[stackStorage->top--];
    }
}

StackElementT *newStackElement(void) {
    StackElementT *stackElement = (StackElementT *)malloc(sizeof(StackElementT));
    stackElement->jsonValue = json_object();
    // Initialize
    stackElement->path = malloc(sizeof(char) * PATH_MAX_LENGTH + 1);
    stackElement->parent = 0;

    return stackElement;
}

void freeStackStorage(StackStorageT *stackStorage) {
    for (int i = 0; i < stackStorage->size; i++) {
        json_decref((*stackStorage->stackElements + i)->jsonValue);
        free((*stackStorage->stackElements + i)->path);
        free(*stackStorage->stackElements + i);
    }
    printf("Freed %i elements on stack", stackStorage->size);
    free(stackStorage);
}

/*Populate jsonKeys with keys obtained from jsonObject*/
void getKeys(json_t *jsonKeys, json_t *jsonObject) {
    json_t *iter = json_object_iter(jsonObject);
    while (iter) {
        const char *key_ = json_object_iter_key(iter);
        json_array_append(jsonKeys, json_string(key_));
        iter = json_object_iter_next(jsonObject, iter);
    }
}

/**
 * Is it even required?
 */
void unwrapValues(json_t *jsonValue) {
    json_t **internalStack = (json_t **)malloc(sizeof(json_t *));

    internalStack[0] = jsonValue;
    int stackSize = 1;

    while (stackSize > 0) {
        json_t *stackJSONValue = internalStack[--stackSize];

        if (json_is_object(stackJSONValue)) {
            const char *jsonKey;
            json_t *jsonValue;

            json_object_foreach(stackJSONValue, jsonKey, jsonValue) {
                internalStack = (json_t **)realloc(internalStack, (stackSize + 1) * sizeof(json_t *));
                internalStack[stackSize++] = jsonValue;
            }
        } else if (json_is_array(stackJSONValue)) {
            size_t index;
            json_t *jsonValue;

            json_array_foreach(stackJSONValue, index, jsonValue) {
                internalStack = (json_t **)realloc(internalStack, (stackSize + 1) * sizeof(json_t *));
                internalStack[stackSize++] = jsonValue;
            }
        }
    }

    free(internalStack);
}

void print_json_object(json_t *json) {
    // Check if the JSON object is null.
    if (json == NULL) {
        return;
    }
    // Print the type of the JSON object.
    switch (json_typeof(json)) {
        case JSON_NULL:
            printf("NULL,\n");
            break;
        case JSON_FALSE:
            printf("FALSE,\n");
            break;
        case JSON_TRUE:
            printf("TRUE,\n");
            break;
        case JSON_INTEGER:
            printf("%.1f,\n", json_number_value(json));
            break;
        case JSON_STRING:
            printf("\"%s\",\n", json_string_value(json));
            break;
        case JSON_ARRAY:
            printf("[\n");
            for (int i = 0; i < (int)json_array_size(json); i++) {
                print_json_object(json_array_get(json, i));
            }
            printf("]\n");
            break;
        case JSON_OBJECT:
            printf("{\n");
            for (json_t *iter = json_object_iter(json); iter; iter = json_object_iter_next(json, iter)) {
                const char *key = json_object_iter_key(iter);
                json_t *value = json_object_iter_value(iter);
                printf("\t \"%s\": ", key);
                print_json_object(value);
            }
            printf("}\n");
            break;
        default:
            fprintf(stderr, "Unhandled JSON Type");
            break;
    }
}

// Assume long value is 64bit long,
void long2str(char *stringValue, long longValue) { sprintf(stringValue, "%ld", longValue); }

/**
 * Lookup SID
 */

// Assume *path = "/" and parent = 0 when this function is being invoked
void lookupSID(json_t *jsonValue, SIDModelT *sidModel) {
    // Look at this exampel to allcate, tag and deallocate tags & items
    // https://bard.google.com/chat/89f043aa4b2be5a1

    // Initial Values
    char *path = "/";
    long parent = 0;

    StackStorageT *stackStorage = (StackStorageT *)malloc(sizeof(StackStorageT));
    initStackStorage(stackStorage, MAX_STACK_SIZE);

    StackElementT *initialStackElement = newStackElement();
    // This element will be used in the while loop to refer current element of
    // the stack
    StackElementT *currentStackElement;

    initialStackElement->jsonValue = jsonValue;
    initialStackElement->path = path;
    initialStackElement->parent = parent;

    char *currentPath = NULL;
    int currentParent = 0;

    push(stackStorage, initialStackElement);
    while (!isEmpty(stackStorage)) {
        currentStackElement = pop(stackStorage);
        currentPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
        strcpy(currentPath, currentStackElement->path);
        currentParent = currentStackElement->parent;

        if (json_is_object(currentStackElement->jsonValue)) {
            const char *key;
            json_t *value;
            char *identityRefStringValue = NULL;

            // TODO Improve the logic, you ideally should never need to do this.
            // Get a list of keys We are doing this since we are mutating json_t
            // object
            json_t *jsonKeys = json_array();
            getKeys(jsonKeys, currentStackElement->jsonValue);

            for (int i = 0; i < (int)json_array_size(jsonKeys); i++) {
                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = currentStackElement->jsonValue;
                inStackElement->path = currentStackElement->path;
                inStackElement->parent = inStackElement->parent;

                key = json_string_value(json_array_get(jsonKeys, i));
                value = json_object_get(currentStackElement->jsonValue, key);

                char *qualifiedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
                strcpy(qualifiedPath, currentPath);
                strcat(qualifiedPath, key);

                const IdentifierSIDT *identifierSID =
                    hashmap_get(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = qualifiedPath});
                if (!identifierSID) {
                    fprintf(stderr,
                            "Following qualified path should be in the map but "
                            "wasn't found %s\n",
                            qualifiedPath);
                    // free(identifierSID);
                    return;
                }

                long childSIDValue = identifierSID->sid;
                long sidDiff = childSIDValue - currentParent;
                char sidKey[SID_KEY_SIZE] = "";
                long2str(sidKey, sidDiff);

                // Set the new parent
                inStackElement->parent = childSIDValue;

                // If value is not 0 (0 is return for success), then fprintf
                if (json_object_set(inStackElement->jsonValue, sidKey, value) < 0)
                    fprintf(stderr, "Couldn't update the json value");

                // Delete the existing key, equivalent to pop() line 160 in
                // pycoreconf.
                if (json_object_del(inStackElement->jsonValue, key) < 0)
                    fprintf(stderr,
                            "Key %s was not found in the json, this should not "
                            "happen\n",
                            key);

                // Before pushing to stack, let the stack element point to the
                // internal value of currentStackElement->jsonValue, line
                // 160-161 pycoreconf.py
                inStackElement->jsonValue = json_object_get(currentStackElement->jsonValue, sidKey);
                // NOTE Check if inStackElement->jsonValue is a string, and if yes,
                // convert it into a long

                if (json_is_string(inStackElement->jsonValue)) {
                    // Remove the leading ":" from the identityRefStringValue
                    identityRefStringValue = getSubstringAfterLastColon(json_string_value(inStackElement->jsonValue));

                    // find SID of the identifier from the map
                    const IdentifierSIDT *identifierSID = hashmap_get(
                        sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = identityRefStringValue});
                    if (!identifierSID) {
                        fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
                        continue;
                    }

                    // TODO
                    // This is a hack because as soon as I point
                    // currentStackElement->jsonValue = json_integer(identifierSID->sid);
                    // it corrupts the json reference and the model is not updated.

                    // json_decref(currentStackElement->jsonValue);
                    // json_integer_set(currentStackElement->jsonValue,
                    // identifierSID->sid);
                    inStackElement->jsonValue = json_integer(identifierSID->sid);
                    json_object_set(currentStackElement->jsonValue, sidKey, inStackElement->jsonValue);
                }

                // Adding the trailing slash and copy the content to
                // inStackElement
                strcat(qualifiedPath, "/");
                inStackElement->path = malloc(sizeof(char) * PATH_MAX_LENGTH);
                strcpy(inStackElement->path, qualifiedPath);
                push(stackStorage, inStackElement);

                // Clean up temporary path
                free(qualifiedPath);
            }

            json_decref(jsonKeys);
        }
        // If the currentStackElement->jsonValue is type of a json list
        else if (json_is_array(currentStackElement->jsonValue)) {
            size_t arrayLength = json_array_size(currentStackElement->jsonValue);
            for (int i = 0; i < (int)arrayLength; i++) {
                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = json_array_get(currentStackElement->jsonValue, i);
                inStackElement->parent = currentStackElement->parent;
                inStackElement->path = currentStackElement->path;
                push(stackStorage, inStackElement);
            }
        }

        // If the currentStackElement->jsonValue is a leaf (such as JSONString)
        else {
            char *identityRefStringValue = NULL;
            char *formattedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
            removeTrailingSlashFromPath(currentStackElement->path, formattedPath);

            const IdentifierTypeT *identifierType =
                hashmap_get(sidModel->identifierTypeHashMap, &(IdentifierTypeT){.identifier = formattedPath});
            //  /ietf-schc:schc/rule/entry/field-length HAS Type as an json array.

            // Check if formattedPath has the type identityref
            if (identifierType == NULL) {
                fprintf(stderr,
                        "No basic types or identityref type found for the Identifier "
                        "path %s\n",
                        formattedPath);
                free(formattedPath);

                // if currentStackElement->jsonValue is a Json String, then try to find
                // its SID value and update it
                if (json_is_string(currentStackElement->jsonValue)) {
                    // Remove the leading ":" from the identityRefStringValue
                    identityRefStringValue =
                        getSubstringAfterLastColon(json_string_value(currentStackElement->jsonValue));

                    // If identityRefStringValue is NULL or if its same as the
                    // formattedPath
                    if (!identityRefStringValue ||
                        !(strcmp(identityRefStringValue, json_string_value(currentStackElement->jsonValue)))) {
                        fprintf(stderr,
                                "No basic types or identityref type found for the Identifier "
                                "path %s\n",
                                formattedPath);
                        free(formattedPath);
                        continue;
                    }
                    // find SID of the identifier from the map
                    const IdentifierSIDT *identifierSID = hashmap_get(
                        sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = identityRefStringValue});
                    if (!identifierSID) {
                        fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
                        continue;
                    }

                    // TODO
                    // This is a hack because as soon as I point
                    // currentStackElement->jsonValue = json_integer(identifierSID->sid);
                    // it corrupts the json reference and the model is not updated.

                    char sidString_[SID_KEY_SIZE];
                    snprintf(sidString_, SID_KEY_SIZE, "%" PRId64, (uint64_t)identifierSID->sid);

                    json_string_set(currentStackElement->jsonValue, sidString_);

                    // json_decref(currentStackElement->jsonValue);
                    // json_integer_set(currentStackElement->jsonValue,
                    // identifierSID->sid); currentStackElement->jsonValue =
                    // json_integer(identifierSID->sid);
                }

                continue;
            }

            switch (identifierType->type) {
                case IDENTITY_REF:
                    // Check if currentStackElement->jsonValue is a json string
                    if (!json_is_string(currentStackElement->jsonValue)) {
                        print_json_object(currentStackElement->jsonValue);
                        fprintf(stderr,
                                "Expected a json string for the Leaf Node with *identityref* "
                                "type %s "
                                "but found something else\n",
                                formattedPath);
                        free(formattedPath);
                        continue;
                    }

                    // Remove the leading ":" from the identityRefStringValue
                    identityRefStringValue =
                        getSubstringAfterLastColon(json_string_value(currentStackElement->jsonValue));

                    // If identityRefStringValue is NULL or if its same as the formattedPath
                    if (!identityRefStringValue ||
                        !(strcmp(identityRefStringValue, json_string_value(currentStackElement->jsonValue)))) {
                        fprintf(stderr,
                                "No basic types or identityref type found for the Identifier "
                                "path %s\n",
                                formattedPath);
                        free(formattedPath);
                        continue;
                    }
                    // find SID of the identifier from the map
                    const IdentifierSIDT *identifierSID = hashmap_get(
                        sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = identityRefStringValue});
                    if (!identifierSID) {
                        fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
                        continue;
                    }

                    // TODO
                    // This is a hack because as soon as I point
                    // currentStackElement->jsonValue = json_integer(identifierSID->sid); it
                    // corrupts the json reference and the model is not updated. value

                    char sidString_[SID_KEY_SIZE];
                    snprintf(sidString_, SID_KEY_SIZE, "%" PRId64, (uint64_t)identifierSID->sid);
                    json_string_set(currentStackElement->jsonValue, sidString_);

                    // json_decref(currentStackElement->jsonValue);
                    // currentStackElement->jsonValue = json_real(identifierSID->sid);
                    break;
                default:
                    break;
            }

            if (identifierType == NULL) {
                fprintf(stderr,
                        "No valid type found for the Identifier "
                        "path %s\n",
                        formattedPath);
                free(formattedPath);
                continue;
            }

            free(formattedPath);
            // TODO FRee Handle JSON VALUES;
        }
    }

    // printf("Finalment: \n");
    // print_json_object(jsonValue);
    // printf("---------\n");
    //  Clean up
    free(currentPath);
    freeStackStorage(stackStorage);
}

/*
Build Coreconf Model from json_t Object and SIDModel like lookupSID
*/

CoreconfValueT *buildCoreconfModelFromJson(json_t *jsonValue, SIDModelT *sidModel, char *path, int64_t parentSID) {
    // Check type of jsonValue
    if (json_is_object(jsonValue)) {
        CoreconfValueT *coreconfValueHashMap = createCoreconfHashmap();
        // Iterate through jsonValue and build the CoreconfHashMap
        const char *key;
        json_t *value;
        json_object_foreach(jsonValue, key, value) {
            // Get fully qualified path
            char *qualifiedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
            strcpy(qualifiedPath, path);
            strcat(qualifiedPath, key);

            // Get the SID value
            const IdentifierSIDT *identifierSID =
                hashmap_get(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = qualifiedPath});
            if (!identifierSID) {
                fprintf(stderr, "Following qualified path should be in the map but wasn't found %s\n", qualifiedPath);
                return NULL;
            }

            // Append a trailing slash
            strcat(qualifiedPath, "/");

            // Get the value
            CoreconfValueT *coreconfValue =
                buildCoreconfModelFromJson(value, sidModel, qualifiedPath, identifierSID->sid);

            // Add the value to the coreconfHashMap
            uint64_t sidDiff = identifierSID->sid - parentSID;
            insertCoreconfHashMap(coreconfValueHashMap->data.map_value, sidDiff, coreconfValue);
        }
        return coreconfValueHashMap;
    } else if (json_is_array(jsonValue)) {
        // Iterate throught the jsonValue
        size_t arrayLength = json_array_size(jsonValue);
        CoreconfValueT *coreconfArrayValue = createCoreconfArray();
        for (int i = 0; i < (int)arrayLength; i++) {
            json_t *arrayElement = json_array_get(jsonValue, i);
            CoreconfValueT *coreconfValue = buildCoreconfModelFromJson(arrayElement, sidModel, path, parentSID);
            addToCoreconfArray(coreconfArrayValue, coreconfValue);
        }
        return coreconfArrayValue;
    } else {
        // Leaves
        char *identityRefStringValue = NULL;
        char *formattedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
        removeTrailingSlashFromPath(path, formattedPath);

        const IdentifierTypeT *identifierType =
            hashmap_get(sidModel->identifierTypeHashMap, &(IdentifierTypeT){.identifier = formattedPath});

        if (identifierType == NULL || identifierType->type == IDENTITY_REF) {
            fprintf(stderr, "No basic types or identityref type found for the Identifier path %s\n", formattedPath);

            // Check if this leaf is a JSON String,
            // if yes, check if there's a corresponding SID value for this leaf
            if (json_is_string(jsonValue)) {
                // Remove the leading ":" from the identityRefStringValue
                identityRefStringValue = getSubstringAfterLastColon(json_string_value(jsonValue));

                // find SID of the identifier from the map
                const IdentifierSIDT *identifierSID = hashmap_get(
                    sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = identityRefStringValue});
                if (identifierSID) {
                    return createCoreconfInteger(identifierSID->sid);
                }
                fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
            }
            return mapJSONToCoreconfValue(jsonValue);
        }

        // Switch from identifierType and create values accordingly
        switch (identifierType->type) {
            case STRING:
                return createCoreconfString(json_string_value(jsonValue));
            case UINT_8:
                return createCoreconfInteger((uint64_t)json_integer_value(jsonValue));
            case UINT_16:
                return createCoreconfInteger((uint64_t)json_integer_value(jsonValue));
            case UINT_32:
                return createCoreconfInteger((uint64_t)json_integer_value(jsonValue));
            case UINT_64:
                return createCoreconfInteger((uint64_t)json_integer_value(jsonValue));
            case DECIMAL64:
                return createCoreconfReal(json_real_value(jsonValue));
            case BOOLEAN:
                // return createCoreconfBoolean(json_boolean_value(jsonValue));
                // TODO Remove this and revert to line above
                return createCoreconfString(json_string_value(jsonValue));
            default:
                fprintf(stderr, "Unhandled Identifier Type");
                return NULL;
        }
    }
}

/*
Given a CORECONF representation of datamodel, and a SID, traverse through the
model and return the entire {sid:value} Assume keys will be a integer array for
now
*/
json_t *traverseCORECONF(json_t *coreconfModel, int64_t sid) {
    StackStorageT *stackStorage = (StackStorageT *)malloc(sizeof(StackStorageT));
    initStackStorage(stackStorage, MAX_STACK_SIZE);

    // TODO Should this be a new Struct type?
    StackElementT *initialStackElement = newStackElement();
    // Top of the stack
    StackElementT *currentStackElement;

    initialStackElement->jsonValue = coreconfModel;
    // Save Delta
    initialStackElement->delta = 0;
    initialStackElement->parent = 0;

    // return value
    json_t *returnValue = json_object();

    // Base SID used during recalculation
    char parentSID[SID_KEY_SIZE] = "";

    push(stackStorage, initialStackElement);
    while (!isEmpty(stackStorage)) {
        currentStackElement = pop(stackStorage);

        if (json_is_object(currentStackElement->jsonValue)) {
            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char keyString[SID_KEY_SIZE];
                snprintf(keyString, SID_KEY_SIZE, "%" PRId64, (uint64_t)sid);
                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }

            /* obj is a JSON object */
            const char *keyString;
            json_t *value;
            json_object_foreach(currentStackElement->jsonValue, keyString, value) {
                // If parentSID is NULL then get the first
                if (!strcmp(parentSID, "")) {
                    strcpy(parentSID, keyString);
                    // parentSID = (char *) keyString;
                }

                // if key matches then return the value
                int64_t deltaKey = char2int64((char *)keyString);
                // If deltaKey is INTMAX_MIN then an error has occured
                if (deltaKey == INTMAX_MIN) return NULL;

                int64_t currentElementSID = deltaKey + currentStackElement->parent;

                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = value;
                inStackElement->parent = currentElementSID;
                push(stackStorage, inStackElement);
            }
        } else if (json_is_array(currentStackElement->jsonValue)) {
            // Can you have list of models as the outermost model?
            if (!strcmp(parentSID, "")) {
                snprintf(parentSID, SID_KEY_SIZE, "%" PRId64, (uint64_t)currentStackElement->parent);
            }

            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char keyString[SID_KEY_SIZE];
                snprintf(keyString, SID_KEY_SIZE, "%" PRId64, (uint64_t)sid);

                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }

            size_t arrayLength = json_array_size(currentStackElement->jsonValue);
            for (int i = 0; i < (int)arrayLength; i++) {
                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = json_array_get(currentStackElement->jsonValue, i);
                inStackElement->parent = currentStackElement->parent;
                push(stackStorage, inStackElement);
            }
        } else {
            // LEAVES
            // Can you have list of models as the outermost model?
            if (!strcmp(parentSID, "")) {
                snprintf(parentSID, SID_KEY_SIZE, "%" PRId64, (uint64_t)currentStackElement->parent);
            }

            // Adjust the SID
            // currentStackElement->jsonValue

            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char keyString[SID_KEY_SIZE];
                snprintf(keyString, SID_KEY_SIZE, "%" PRId64, (uint64_t)sid);

                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }
        }
    }
    // TODO free stackStorage
    // TODO free keyString, parentSID
    return NULL;
}

json_t *traverseCORECONFWithKeys(json_t *jsonInstance, SIDModelT *sidModel, IdentifierSIDT *sidIdentifier, int keys[],
                                 size_t keyLength) {
    int64_t sid = INT64_MIN;
    // If SID is passed as an argument then update the local sid variable
    if (sidIdentifier->sid != sid) {
        sid = sidIdentifier->sid;
    } else if (sidIdentifier->identifier) {
        // If identifier is passed as an argument then find the corresponding SID
        // and update the variable Change all instances of ":" to "/" in the
        // identifier, lets call it formattedIdentifier, before querying for the SID

        char *formattedIdentifier = malloc(sizeof(char) * (strlen(sidIdentifier->identifier) + 1));
        strcpy(formattedIdentifier, sidIdentifier->identifier);
        char *colon = strchr(formattedIdentifier, ':');
        while (colon != NULL) {
            *colon = '/';
            colon = strchr(colon, ':');
        }
        const IdentifierSIDT *foundIdentiferSID =
            hashmap_get(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = formattedIdentifier});

        // Clean up any memory allocated
        free(formattedIdentifier);

        if (foundIdentiferSID)
            sid = foundIdentiferSID->sid;
        else
            fprintf(stderr, "No SID found for the following identifier %s\n", sidIdentifier->identifier);
    }

    // Check if the SID to be queried is valid, if not, then return
    if (sid == INT64_MIN) {
        fprintf(stderr, "Valid SID %d is not found!\n", (int)sid);
        return NULL;
    }

    // call getCCORECONF
    json_t *returnValue = getCCORECONF(jsonInstance, sidModel, sid, keys, keyLength, 0, 0, NULL);

    return returnValue;
}

json_t *getCCORECONF(json_t *coreconfModel, SIDModelT *sidModel, int sid, int keys[], size_t keyLength, int delta,
                     int depth, json_t *value) {
    if (sid == delta) {
        // If value is NONE
        if ((value == NULL) || (json_object_size(value) == 0)) return coreconfModel;

        // If value is not NONE
        coreconfModel = value;
        // XXX return True in python
        return coreconfModel;
    }

    // If the coreconfModel is not an object, then return NULL
    if (!json_is_object(coreconfModel)) {
        return NULL;
    }

    // Henceforth coreconfModel is only a JSON object
    json_t *result = json_object();
    int sidDiff = sid - delta;

    // check if sidDiff is one of the keys in the coreconfModel
    char sidDiffString[SID_KEY_SIZE];
    snprintf(sidDiffString, SID_KEY_SIZE, "%" PRId64, (uint64_t)sidDiff);

    json_t *sidDiffValue = json_object_get(coreconfModel, sidDiffString);

    // If no keys are given and sidDiff is part of coreconfModel, then:
    if ((keyLength == 0) && (sidDiffValue != NULL)) {
        if (value) {
            json_object_set(coreconfModel, sidDiffString, value);
            return coreconfModel;
        } else {
            json_object_set(result, sidDiffString, sidDiffValue);
            return result;
        }
    }

    // If no keys are given and value is not NULL, then:
    if ((keyLength == 0) && (value != NULL)) {
        json_object_set(coreconfModel, sidDiffString, value);
        return coreconfModel;
    }

    // Iterate through coreconfModel for all keys and values
    const char *keyString;
    json_t *keyValue;
    json_object_foreach(coreconfModel, keyString, keyValue) {
        // convert keyValue from const char* to int64_t
        int64_t keyValue_int64 = char2int64((char *)keyString);

        if (keyValue_int64 == INTMAX_MIN) return NULL;

        int64_t newSID = keyValue_int64 + delta;

        // check if newSID is in sidModel->keyMappingHashMap, if not found, then
        // return NULL
        const KeyMappingT *keyMapping = hashmap_get(sidModel->keyMappingHashMap, &(KeyMappingT){.key = newSID});

        if (keyMapping) {
            // newSID exists in sid key mapping

            // check if keyLength is equal to keyMapping->dynamicLongList->size, if
            // not equal, then return NULL
            if (keyLength < keyMapping->dynamicLongList->size) {
                fprintf(stderr,
                        "Length of keys is not the same as key-mapping found "
                        "in .sid file\n");
                return NULL;
            }

            // XXX Should keySearchObject be an internal hashmap?
            json_t *keySearchObject = json_object();
            // Iterate through dynamicLongList to build keySearchObject
            char newDeltaString[SID_KEY_SIZE];

            for (int i = 0; i < (int)keyMapping->dynamicLongList->size; i++) {
                int64_t mappedKey = *(keyMapping->dynamicLongList->longList + i);
                int64_t newDelta = mappedKey - newSID;
                // XXX pop value from keys
                int64_t sidKey = *(keys + i);
                snprintf(newDeltaString, SID_KEY_SIZE, "%" PRId64, (uint64_t)newDelta);
                json_object_set(keySearchObject, newDeltaString, json_integer(sidKey));
            }

            json_t *foundST = json_object();
            int foundIndex = 0;
            if (!json_is_array(keyValue)) {
                fprintf(stderr, "keyValue is not an object. This shouldn't happen\n");
                return NULL;
            }

            /* keyValue is a JSON array */
            size_t leafIndex;
            json_t *leafValue;
            json_array_foreach(keyValue, leafIndex, leafValue) {
                if (json_object_size(keySearchObject) != json_object_size(leafValue)) {
                    foundST = leafValue;
                    break;
                }
                foundIndex += 1;
            }

            if (json_object_size(foundST) != 0) {
                if (sid == newSID) {
                    if ((value != NULL) && (json_object_size(value) != 0)) {
                        json_array_set(keyValue, foundIndex, value);
                        return coreconfModel;
                    }
                    // If value is NULL, then create a new json_t object stDeltaAdjusted
                    // and populate it with key, value from foundST
                    json_t *stDeltaAdjusted = json_object();
                    const char *key1;
                    json_t *value1;
                    json_object_foreach(foundST, key1, value1) {
                        int64_t key1_int64 = char2int64((char *)key1);
                        int64_t adjustedDelta = key1_int64 + newSID;
                        char adjustedDeltaString[SID_KEY_SIZE];
                        snprintf(adjustedDeltaString, SID_KEY_SIZE, "%" PRId64, (uint64_t)adjustedDelta);
                        json_object_set(stDeltaAdjusted, adjustedDeltaString, value1);
                    }
                    return stDeltaAdjusted;
                }

                return getCCORECONF(foundST, sidModel, sid, keys, keyLength, newSID, depth + 1, value);
            }

            // if foundST is not NULL or empty, then check for value
            if ((value != NULL) && (json_object_size(value) != 0)) {
                printf("Add it \n");
                print_json_object(keySearchObject);
                json_t *keySearchObjectCopy = json_copy(keySearchObject);
                // Iterate over value and set it to keySearchObjectCopy
                const char *updateKey;
                json_t *updateValue;
                json_object_foreach(value, updateKey, updateValue) {
                    if (json_object_get(keySearchObjectCopy, updateKey) != NULL) {
                        printf("Key already exists and is already set %s", updateKey);
                    } else {
                        json_object_set(keySearchObjectCopy, updateKey, updateValue);
                    }
                }
                json_array_append(keyValue, keySearchObjectCopy);
                return coreconfModel;
            }
        } else {  // if newSID is not found in the keyMappingHashMap
            if (json_object_size(result) == 0)
                result = getCCORECONF(keyValue, sidModel, sid, keys, keyLength, newSID, depth + 1, value);
        }
    }

    if (json_object_size(result) != 0) return result;

    return NULL;
}
