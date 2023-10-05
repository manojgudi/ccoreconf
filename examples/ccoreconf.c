#include "ccoreconf.h"
#include "hashmap.h"
#include <cbor.h>
#include <jansson.h>
#include <libyang/libyang.h>
#include <stdint.h>
#include <stdlib.h>

#define PATH_MAX_LENGTH 100
#define MAX_STACK_SIZE 100
#define SID_KEY_SIZE 21

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
        printf("%f,\n", json_number_value(json));
        break;
    case JSON_STRING:
        printf("\"%s\",\n", json_string_value(json));
        break;
    case JSON_ARRAY:
        printf("[\n");
        for (int i = 0; i < json_array_size(json); i++) {
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
    cbor_item_t *CBOR_SID_TAG = cbor_new_tag(47);

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

            // TODO Improve the logic, you ideally should never need to do this.
            // Get a list of keys We are doing this since we are mutating json_t
            // object
            json_t *jsonKeys = json_array();
            getKeys(jsonKeys, currentStackElement->jsonValue);

            for (int i = 0; i < json_array_size(jsonKeys); i++) {

                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = currentStackElement->jsonValue;
                inStackElement->path = currentStackElement->path;
                inStackElement->parent = inStackElement->parent;

                key = json_string_value(json_array_get(jsonKeys, i));
                value = json_object_get(currentStackElement->jsonValue, key);

                char *qualifiedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
                strcpy(qualifiedPath, currentPath);
                strcat(qualifiedPath, key);

                IdentifierSIDT *identifierSID =
                    hashmap_get(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = qualifiedPath});
                if (!identifierSID) {
                    fprintf(stderr,
                            "Following qualified path should be in the map but "
                            "wasn't found %s\n",
                            qualifiedPath);
                    free(identifierSID);
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
            for (int i = 0; i < arrayLength; i++) {
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

            IdentifierTypeT *identifierType =
                hashmap_get(sidModel->identifierTypeHashMap, &(IdentifierTypeT){.identifier = formattedPath});
            //  /ietf-schc:schc/rule/entry/field-length HAS Type as an json array.

            // Check if formattedPath has the type identityref
            if (identifierType == NULL) {

                fprintf(stderr,
                        "No basic types or identityref type found for the Identifier "
                        "path %s\n",
                        formattedPath);
                free(formattedPath);

                // if currentStackElement->jsonValue is a Json String, then try to find its SID value and update it
                if (json_is_string(currentStackElement->jsonValue)) {
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
                    IdentifierSIDT *identifierSID = hashmap_get(
                        sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = identityRefStringValue});
                    if (!identifierSID) {
                        fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
                        free(identifierSID);
                        continue;
                    }

                    // TODO
                    // This is a hack because as soon as I point currentStackElement->jsonValue =
                    // json_integer(identifierSID->sid); it corrupts the json reference and the model is not updated.
                    // value
                    char *sidString_ = malloc(sizeof(char) * SID_KEY_SIZE);
                    json_string_set(currentStackElement->jsonValue, int2str(sidString_, identifierSID->sid));
                    free(sidString_);
                }

                continue;
            }

            switch (identifierType->type) {
            case IDENTITY_REF:
                // Check if currentStackElement->jsonValue is a json string
                if (!json_is_string(currentStackElement->jsonValue)) {
                    fprintf(stderr,
                            "Expected a json string for the Leaf Node with *identityref* type %s "
                            "but found something else\n",
                            formattedPath);
                    free(formattedPath);
                    continue;
                }

                // Remove the leading ":" from the identityRefStringValue
                identityRefStringValue = getSubstringAfterLastColon(json_string_value(currentStackElement->jsonValue));

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
                IdentifierSIDT *identifierSID = hashmap_get(sidModel->identifierSIDHashMap,
                                                            &(IdentifierSIDT){.identifier = identityRefStringValue});
                if (!identifierSID) {
                    fprintf(stderr, "No SID found for the following identifier %s\n", identityRefStringValue);
                    free(identifierSID);
                    continue;
                }

                // TODO
                // This is a hack because as soon as I point currentStackElement->jsonValue =
                // json_integer(identifierSID->sid); it corrupts the json reference and the model is not updated. value
                char *sidString_ = malloc(sizeof(char) * SID_KEY_SIZE);
                json_string_set(currentStackElement->jsonValue, int2str(sidString_, identifierSID->sid));
                free(sidString_);
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

void convertToCBORType(json_t *jsonItem, enum SchemaIdentifierTypeEnum identifierType, cbor_item_t *cborItem,
                       SIDModelT *sidModel) {
    size_t jsonStringLength;
    switch (identifierType) {
    // For null terminated UTF-8 string
    case STRING:
        // Deallocate previous cborItem as cbor_build_stringn allocates new
        // memory
        free(cborItem);
        cborItem = NULL;
        const char *jsonValue = json_string_value(jsonItem);
        jsonStringLength = json_string_length(jsonItem);
        cborItem = cbor_build_stringn(jsonValue, jsonStringLength);
        break;
    case UINT_8:
        uint8_t jsonValue_8i = (uint8_t)json_integer_value(jsonItem);
        cbor_set_uint8(cborItem, jsonValue_8i);
        break;
    case UINT_16:
        uint16_t jsonValue_16i = (uint16_t)json_integer_value(jsonItem);
        cbor_set_uint16(cborItem, jsonValue_16i);
        break;
    case UINT_32:
        uint32_t jsonValue_32i = (uint32_t)json_integer_value(jsonItem);
        cbor_set_uint32(cborItem, jsonValue_32i);
        break;
    case UINT_64:
        uint64_t jsonValue_64i = (uint64_t)json_integer_value(jsonItem);
        cbor_set_uint64(cborItem, jsonValue_64i);
        break;
    case BOOLEAN:
        bool jsonValue_b = json_boolean_value(jsonItem);
        cbor_set_bool(cborItem, jsonValue_b);
    case IDENTITY_REF:
        // Tag 47 is for YANG Schema Identifier
        // change 12 to its value
        cbor_tag_set_item(cbor_new_tag(47), (uint64_t)12);
        break;
    default:
        // Treat it as a string
        // Deallocate previous cborItem as cbor_build_stringn allocates new
        // memory
        free(cborItem);
        cborItem = NULL;
        const char *jsonValue_d = json_string_value(jsonItem);
        jsonStringLength = json_string_length(jsonItem);
        cborItem = cbor_build_stringn(jsonValue_d, jsonStringLength);
        break;
    }
}

// use an indefinite map to create a new cbormap
// cbor_item_t *cborIMap = cbor_new_indefinite_map();
void convertToCORECONF(cbor_item_t *cborIMap, json_t *jsonMap) {
    if (!json_is_object(jsonMap))
        return;

    // Iterate over the map
}

/*
This function is used to recalculate the SID of coreconfModel (typically a
subset of the entire coreconfModel) parentSID is the initial SID for the
coreconfModel, tree

void recalculateSIDs(json_t *coreconfModel, int64_t parentSID, int64_t baseSID){
    StackStorageT *stackStorage = (StackStorageT *)
malloc(sizeof(StackStorageT)); initStackStorage(stackStorage, MAX_STACK_SIZE);

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
    char *parentSID = NULL;
    int delta = baseSID - parentSID;

    push(stackStorage, initialStackElement);
    while (!isEmpty(stackStorage)){
        currentStackElement = pop(stackStorage);

        if ( json_is_object(currentStackElement->jsonValue) ){

        } else if (){

    }

}*/

/*
Given a CORECONF representation of datamodel, and a SID, traverse through the
model and return the entire {sid:value} Assume keys will be a integer array for
now
*/
json_t *traverseCORECONF(json_t *coreconfModel, SIDModelT *sidModel, int64_t sid) {
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
    char *parentSID = NULL;

    push(stackStorage, initialStackElement);
    while (!isEmpty(stackStorage)) {

        currentStackElement = pop(stackStorage);

        if (json_is_object(currentStackElement->jsonValue)) {

            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char *keyString = int2str(keyString, sid);
                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }

            /* obj is a JSON object */
            const char *keyString;
            json_t *value;

            json_object_foreach(currentStackElement->jsonValue, keyString, value) {

                // If parentSID is NULL then get the first
                if (!parentSID) {
                    parentSID = keyString;
                }

                // if key matches then return the value
                int64_t deltaKey = char2int64(keyString);
                // If deltaKey is INTMAX_MIN then an error has occured
                if (deltaKey == INTMAX_MIN)
                    return NULL;

                int64_t currentElementSID = deltaKey + currentStackElement->parent;

                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = value;
                inStackElement->parent = currentElementSID;
                push(stackStorage, inStackElement);
            }
        } else if (json_is_array(currentStackElement->jsonValue)) {

            // Can you have list of models as the outermost model?
            if (!parentSID) {
                parentSID = int2str(parentSID, currentStackElement->parent);
            }

            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char *keyString = int2str(keyString, sid);
                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }

            size_t arrayLength = json_array_size(currentStackElement->jsonValue);
            for (int i = 0; i < arrayLength; i++) {
                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = json_array_get(currentStackElement->jsonValue, i);
                inStackElement->parent = currentStackElement->parent;
                push(stackStorage, inStackElement);
            }
        } else {
            // LEAVES
            // Can you have list of models as the outermost model?
            if (!parentSID) {
                parentSID = int2str(parentSID, currentStackElement->parent);
            }

            // Adjust the SID
            // currentStackElement->jsonValue

            // Check
            if (((int64_t)currentStackElement->parent == sid)) {
                char *keyString = int2str(keyString, sid);
                json_object_set(returnValue, keyString, currentStackElement->jsonValue);
                return returnValue;
            }
        }
    }
    // TODO free stackStorage
    // TODO free keyString, parentSID
}