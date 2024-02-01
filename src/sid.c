#include "../include/sid.h"
#include "../include/hashmap.h"
#include <errno.h>
#include <inttypes.h>
#include <jansson.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SID_LENGTH 20
// Array of string representations for the enum values
const char *SchemaIdentifierTypeStrings[] = {"String",
                                             "Unsigned Integer 8bit",
                                             "Unsigned Integer 16bit",
                                             "Unsigned Integer 32bit",
                                             "Unsigned Integer 64bit",
                                             "RCS Algorithm",
                                             "Decimal 64bit",
                                             "Boolean",
                                             "Identity Ref"};

DynamicLongListT *createDynamicLongList() {
    DynamicLongListT *dynamicLongList = (DynamicLongListT *)malloc(sizeof(DynamicLongListT));

    if (dynamicLongList == NULL) {
        fprintf(stderr, "Failed malloc'ing Dynamic long list");
        return NULL;
    }
    dynamicLongList->longList = NULL;
    dynamicLongList->size = 0;
    return dynamicLongList;
}

void initializeDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, do nothing;
    if (!dynamicLongList)
        return;
    dynamicLongList->longList = malloc(sizeof(long));
    dynamicLongList->size = 0;
}

// Comparison function for qsort
long compareLong(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Function to sort the dynamic long list, it assumes sortedArray is already correctly inititliazed to dynamicLongList->size
void sortDynamicLongList(DynamicLongListT* dynamicLongList, long sortedArray[]){
    // populate the sortedArray with dynamicLongList values
    for (int i = 0; i < dynamicLongList->size; i++) {
        sortedArray[i] = dynamicLongList->longList[i];
    }

    // Sort the array
    qsort(sortedArray, dynamicLongList->size, sizeof(long), compareLong);
    // Add the sorted array to the sortedDynamicLongList
}

// Function to compare two dynamicLongList which converts to a sorted long array and then compares the array
bool compareDynamicLongList(DynamicLongListT* dynamicLongList1, DynamicLongListT* dynamicLongList2){
    size_t array1Size = dynamicLongList1->size;
    size_t array2Size = dynamicLongList2->size;

    // Check if either of the dynamicLongLists have size = 0
    if ((array1Size == 0) || (array2Size == 0))
        return false;

    if (array1Size !=array2Size)
        return false;

    // Created two long arrays for sorting with size of dynamicLongLists
    long array1[array1Size];
    long array2[array1Size];

    // sort the two dynamicLongLists
    sortDynamicLongList(dynamicLongList1, array1);
    sortDynamicLongList(dynamicLongList2, array2);

    // Compare the two arrays, and return true if array1 is exactly the same as array2
    for (int i = 0; i < array1Size; i++){
        if (array1[i] != array2[i])
            return false;
    }

    return true;
}


void addLong(DynamicLongListT *dynamicLongList, long value) {
    size_t currentListSize;
    if (dynamicLongList == NULL) {
        currentListSize = 0;
    } else {
        currentListSize = dynamicLongList->size;
    }
    printDynamicLongList(dynamicLongList);

    dynamicLongList->longList = (long *)realloc(dynamicLongList->longList, (currentListSize + 1) * sizeof(long));
    // Check if realloc happened properly, if no, then realloc failed and longList will be NULL
    if (!dynamicLongList->longList) {
        fprintf(stderr, "Failed realloc'ing long list");
        return;
    }
    dynamicLongList->size = currentListSize + 1;
    dynamicLongList->longList[currentListSize] = value;
    printDynamicLongList(dynamicLongList);
}

// pop the last value from dynamicLongList
long popLong(DynamicLongListT *dynamicLongList) {
    // If its NULL, then do nothing
    if (!dynamicLongList)
        return 0;
    // If the list is empty, then return -1
    if (dynamicLongList->size == 0)
        return 0;
    long lastValue = dynamicLongList->longList[dynamicLongList->size - 1];
    dynamicLongList->longList = (long *)realloc(dynamicLongList->longList, (dynamicLongList->size - 1) * sizeof(long));
    dynamicLongList->size = dynamicLongList->size - 1;
    return lastValue;
}

// Clone a DynamicLongListT
void cloneDynamicLongList(DynamicLongListT *originalDynamicLongList, DynamicLongListT *clonedDynamicLongList){
    // If its NULL, then do nothing
    if (originalDynamicLongList == NULL || originalDynamicLongList->size == 0 )
        return NULL;

    // Initialize the clonedDynamicLongList
    initializeDynamicLongList(clonedDynamicLongList);
    // Iterate over the originalDynamicLongList and add all the values to the clonedDynamicLongList
    for (int i = 0; i < originalDynamicLongList->size; i++) {
        addLong(clonedDynamicLongList,  originalDynamicLongList->longList[i]);
    }
}


void addUniqueLong(DynamicLongListT *dynamicLongList, long value){
    // Check if the value already exists in the list
    for (int i = 0; i < dynamicLongList->size; i++) {
        if (dynamicLongList->longList[i] == value) {
            return;
        }
    }
    addLong(dynamicLongList, value);
}


void printDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, then print []
    if (!dynamicLongList) {
        printf("[]");
        return;
    }
    printf("DLL: ");
    for (int i = 0; i < dynamicLongList->size; i++) {
        printf("%lu, ", dynamicLongList->longList[i]);
    }
    printf("\n");
}

void freeDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, then do nothing
    if (!dynamicLongList)
        return;
    free(dynamicLongList->longList);
    free(dynamicLongList);
}

int keyMappingCompare(const void *a, const void *b, void *udata) {
    const KeyMappingT *keyMapping1 = (KeyMappingT *)a;
    const KeyMappingT *keyMapping2 = (KeyMappingT *)b;
    // return strcmp(keyMapping1->key, keyMapping2->key);
    return (keyMapping1->key != keyMapping2->key);
}

uint64_t keyMappingHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const KeyMappingT *keyMapping = (KeyMappingT *)item;
    // char keyString[SID_LENGTH];
    // sprintf(keyString, "%lu", keyMapping->key);
    //  TODO Can you replace log with something else?
    return hashmap_sip(&keyMapping->key, sizeof(uint64_t), seed0, seed1);
}

int identifierSIDCompare(const void *a, const void *b, void *udata) {
    const IdentifierSIDT *identifierSID1 = (IdentifierSIDT *)a;
    const IdentifierSIDT *identifierSID2 = (IdentifierSIDT *)b;
    return strcmp(identifierSID1->identifier, identifierSID2->identifier);
}

uint64_t identifierSIDHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const IdentifierSIDT *identifierSID = (IdentifierSIDT *)item;
    return hashmap_sip(identifierSID->identifier, strlen(identifierSID->identifier), seed0, seed1);
}

int sidIdentifierCompare(const void *a, const void *b, void *udata) {
    const SIDIdentifierT *sidIdentifier1 = (SIDIdentifierT *)a;
    const SIDIdentifierT *sidIdentifier2 = (SIDIdentifierT *)b;
    return (sidIdentifier1->sid != sidIdentifier2->sid);
}

uint64_t sidIdentifierHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SIDIdentifierT *sidIdentifier = (SIDIdentifierT *)item;
    return hashmap_sip(&sidIdentifier->sid, (sidIdentifier->sid) % 10, seed0, seed1);
}

int identifierTypeCompare(const void *a, const void *b, void *udata) {
    const IdentifierTypeT *identifierType1 = (IdentifierTypeT *)a;
    const IdentifierTypeT *identifierType2 = (IdentifierTypeT *)b;
    return strcmp(identifierType1->identifier, identifierType2->identifier);
}

uint64_t identifierTypeHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const IdentifierTypeT *identifierType = (IdentifierTypeT *)item;
    return hashmap_sip(identifierType->identifier, strlen(identifierType->identifier), seed0, seed1);
}

// NOTE This doesn't work well, idk why, it has to be initialized from the main
void initializeKeyMappingHashMap(struct hashmap *keyMappingHashMap) {
    keyMappingHashMap = hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
}

/*
Allot all memory explicitly in the main
sidFileJSON has the form {"identifierString" : ["identifierKey"] }
need sidModel to convert identifierString to lookup sid numbers
*/

/*
Allot all memory explicitly in the main
*/
void buildKeyMappingHashMap(struct hashmap *keyMappingHashMap, json_t *sidFileJSON) {

    const char *keyMappingString = "key-mapping";
    json_t *keyMappingJSON = json_object_get(sidFileJSON, keyMappingString);

    // Check if key-mapping exists in the SID file
    if (!json_is_object(keyMappingJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return;
    }

    // Iterate over the key-mapping tree to build our own datastructure
    const char *key;
    json_t *value;
    json_object_foreach(keyMappingJSON, key, value) {

        // Create an initialize an an empty list
        DynamicLongListT *dynamicLongList = malloc(sizeof(DynamicLongListT));
        if (dynamicLongList == NULL) {
            fprintf(stderr, "Failed allocating memory to dynamicLongList for the key %s", key);
            continue;
        }
        initializeDynamicLongList(dynamicLongList);

        // Convert key to a long value first
        char *endPtr;
        int64_t parentSID = strtoll(key, &endPtr, 10);
        if (*endPtr != '\0') {
            fprintf(stderr, "Failed converting the key to a int64_t %s", key);
            continue;
        }

        // If the value is not an array then continue without adding that key
        if (!json_is_array(value)) {
            fprintf(stderr, "Following key %s doesn't have valid SID children", key);
            continue;
        }

        for (size_t i = 0; i < json_array_size(value); i++) {
            json_t *childSIDJSON = json_array_get(value, i);
            if (!json_is_integer(childSIDJSON)) {
                fprintf(stderr, "Following value is not a valid SID %s ", json_string_value(childSIDJSON));
                continue;
            }
            // Get the long value
            long childSIDLong = json_integer_value(childSIDJSON);
            // Add value to the dynamicLongList
            addLong(dynamicLongList, childSIDLong);
        }
        // Populate the Hashmap
        hashmap_set(keyMappingHashMap, &(KeyMappingT){.key = parentSID, .dynamicLongList = dynamicLongList});
    }
}

void buildKeyMappingHashMap2(struct hashmap *keyMappingHashMap, json_t *sidFileJSON, SIDModelT* sidModel) {

    const char *keyMappingString = "key-mapping";
    json_t *keyMappingJSON = json_object_get(sidFileJSON, keyMappingString);

    // Check if key-mapping exists in the SID file
    if (!json_is_object(keyMappingJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return;
    }

    // Iterate over the key-mapping tree to build our own datastructure
    const char *unformattedIdentifer;
    json_t *value;
    json_object_foreach(keyMappingJSON, unformattedIdentifer, value) {

        // Create an initialize an an empty list
        DynamicLongListT *dynamicLongList = malloc(sizeof(DynamicLongListT));
        if (dynamicLongList == NULL) {
            fprintf(stderr, "Failed allocating memory to dynamicLongList for the key %s", unformattedIdentifer);
            continue;
        }
        initializeDynamicLongList(dynamicLongList);

        // Change all instances of ":" to "/" in the unformattedIdentifer which is the key
        char *key = malloc((strlen(unformattedIdentifer) + 1) * sizeof(char));
        strcpy(key, unformattedIdentifer);
        char *colon = strchr(key, ':');
        while (colon != NULL) {
            *colon = '/';
            colon = strchr(colon, ':');
        }

        // lookup the key in the sidModel
        // find SID of the identifier from the map
        const IdentifierSIDT *identifierSID = hashmap_get(
            sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = key});
        if (!identifierSID) {
            fprintf(stderr, "No SID found for the following identifier %s\n", key);
            //free(identifierSID);   
        }

        int64_t parentSID = identifierSID->sid;

        // If the value is not an array then continue without adding that key
        if (!json_is_array(value)) {
            fprintf(stderr, "Following key %s doesn't have valid SID children", key);
            continue;
        }

        // Iterate over the array of children
        for (size_t i = 0; i < json_array_size(value); i++) {
            json_t *childSIDJSON = json_array_get(value, i);
            if (!json_is_string(childSIDJSON)) {
                fprintf(stderr, "Following value is not a valid SID %s ", json_string_value(childSIDJSON));
                continue;
            }
            // Get the string value
            const char *childSIDString = json_string_value(childSIDJSON);
            // Replace all instances of ":" with "/" in childSIDString
            char *childSIDStringWithSlash = malloc((strlen(childSIDString) + 1) * sizeof(char));
            strcpy(childSIDStringWithSlash, childSIDString);
            char *colon = strchr(childSIDStringWithSlash, ':');
            while (colon != NULL) {
                *colon = '/';
                colon = strchr(colon, ':');
            }


            // Get the SID value from the sidModel
            const IdentifierSIDT *identifierSIDChild = hashmap_get(
                sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier = childSIDStringWithSlash});
            if (!identifierSIDChild) {
                fprintf(stderr, "No SID found for the following identifier %s\n", childSIDStringWithSlash);
                //free(identifierSIDChild);
            }
            // Get the long value
            long childSIDLong = identifierSIDChild->sid;
            // Add value to the dynamicLongList
            addLong(dynamicLongList, childSIDLong);

        }

    
        // Populate the Hashmap
        hashmap_set(keyMappingHashMap, &(KeyMappingT){.key = parentSID, .dynamicLongList = dynamicLongList});

    }
}

void printKeyMappingT(const KeyMappingT *keyMapping) {
    printf("\nFor the key %" PRIu64 ": \n", keyMapping->key);

    // Iterate over DynamicLongListT
    for (int i = 0; i < keyMapping->dynamicLongList->size; i++) {
        long childSID = *(keyMapping->dynamicLongList->longList + i);
        printf("%lu, ", childSID);
    }
}

void printIdentifierSIDT(const IdentifierSIDT *identifierSID) {
    printf("\nIdentifier %s: %lu (SID) ", identifierSID->identifier, identifierSID->sid);
}

void printSIDIdentifierT(const SIDIdentifierT *sidIdentifier) {
    printf("\nSID %lu: %s (Identifier) ", sidIdentifier->sid, sidIdentifier->identifier);
}

void printIdentifierTypeT(const IdentifierTypeT *identifierType) {
    printf("\nIdentifier %s: %s (type) ", identifierType->identifier,
           SchemaIdentifierTypeStrings[identifierType->type]);
}

void printKeyMappingHashMap(struct hashmap *keyMappingHashMap) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(keyMappingHashMap, &iter, &item)) {
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }
}

void printHashMap(struct hashmap *anyHashMap, enum HashMapTypeEnum hashmapType) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(anyHashMap, &iter, &item)) {
        switch (hashmapType) {
        case KEY_MAPPING:
            const KeyMappingT *keyMapping = item;
            printKeyMappingT(keyMapping);
            break;
        case IDENTIFIER_SID:
            const IdentifierSIDT *identifierSID = item;
            printIdentifierSIDT(identifierSID);
            break;
        case SID_IDENTIFIER:
            const SIDIdentifierT *sidIdentifier = item;
            printSIDIdentifierT(sidIdentifier);
            break;
        case IDENTIFIER_TYPE:
            const IdentifierTypeT *identifierType = item;
            printIdentifierTypeT(identifierType);
            break;
        default:
            fprintf(stderr, "Unknown Hashmap type");
            break;
        }
    }
}


void buildSIDModel(SIDModelT *sidModel, json_t *sidFileJSON) {
    const char *itemsString = "items";
    json_t *itemsJSON = json_object_get(sidFileJSON, itemsString);

    // Check if itemsString exists in the SID file and if its correctly parsed
    if (!json_is_array(itemsJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON array:", itemsString);
        return;
    }

    // Iterate through the "items" containing JSON Maps
    size_t itemsSize = json_array_size(itemsJSON);
    for (size_t i = 0; i < itemsSize; i++) {
        json_t *itemMap = json_array_get(itemsJSON, i);
        if (json_is_object(itemMap)) {
            json_t *sidJSON = json_object_get(itemMap, "sid");
            // convert SID JSON to long
            if (!json_is_integer(sidJSON)) {
                fprintf(stderr, "Following item at %zu does not have a valid SID", i);
                continue;
            }
            long sid = json_integer_value(sidJSON);

            json_t *identifierJSON = json_object_get(itemMap, "identifier");
            // convert identifier to char *
            if (!json_is_string(identifierJSON)) {
                fprintf(stderr, "Following item at %zu does not have a valid identifier", i);
                continue;
            }
            const char *identifier = json_string_value(identifierJSON);

            // Set both the maps
            hashmap_set(sidModel->sidIdentifierHashMap,
                        &(SIDIdentifierT){.sid = sid, .identifier = (char *)identifier});

            hashmap_set(sidModel->identifierSIDHashMap,
                        &(IdentifierSIDT){.sid = sid, .identifier = (char *)identifier});

            // typeJSON can be optional
            json_t *typeJSON = json_object_get(itemMap, "type");

            // TODO Handle list types
            if (typeJSON != NULL && json_is_string(typeJSON)) {
                const char *typeString = json_string_value(typeJSON);
                enum SchemaIdentifierTypeEnum type;
                // Convert typeJSON to one of the enum
                if (!strcmp(typeString, "boolean"))
                    type = BOOLEAN;
                else if (!strcmp(typeString, "binary"))
                    type = BOOLEAN;
                else if (!strcmp(typeString, "string"))
                    type = STRING;
                else if (!strcmp(typeString, "decimal64"))
                    type = DECIMAL64;
                else if (!strcmp(typeString, "uint8"))
                    type = UINT_8;
                else if (!strcmp(typeString, "uint16"))
                    type = UINT_16;
                else if (!strcmp(typeString, "uint32"))
                    type = UINT_32;
                else if (!strcmp(typeString, "uint64"))
                    type = UINT_64;
                else if (!strcmp(typeString, "rcs-algorithm-type"))
                    type = RCS_ALGORITHM;
                else if (!strcmp(typeString, "identityref"))
                    type = IDENTITY_REF;
                else {
                    fprintf(stderr, "Unknown Identifier type, assigning type as STRING: %s\n", typeString);
                    type = STRING;
                    // continue;
                }
                hashmap_set(sidModel->identifierTypeHashMap,
                            &(IdentifierTypeT){.identifier = (char *)identifier, .type = type});
            }
        }
    }
}

void buildSIDModel2(SIDModelT *sidModel, json_t *sidFileJSON) {
    const char *itemsString = "items";
    json_t *itemsJSON = json_object_get(sidFileJSON, itemsString);

    // Check if itemsString exists in the SID file and if its correctly parsed
    if (!json_is_array(itemsJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON array:", itemsString);
        return;
    }

    // Get the first item from itemsJSON and extract its identifier, prefix it with "/" and add it to the sidIdentifierHashmap and identifierSIDHashmap
    json_t *firstItemMap = json_array_get(itemsJSON, 0);
    if (json_is_object(firstItemMap)) {
        json_t *identifierJSON = json_object_get(firstItemMap, "identifier");
        // convert identifier to char *
        if (!json_is_string(identifierJSON)) {
            fprintf(stderr, "Following item at 0 does not have a valid identifier");
            return;
        }
        const char *identifier = json_string_value(identifierJSON);
        // Add "/" to the identifier
        char *identifierWithSlash = malloc((strlen(identifier) + 2) * sizeof(char));
        strcpy(identifierWithSlash, "/");
        strcat(identifierWithSlash, identifier);

        // Get the SID value
        json_t *firstSIDJSON = json_object_get(firstItemMap, "sid");
        // convert SID JSON to long
        if (!json_is_integer(firstSIDJSON)) {
            fprintf(stderr, "Following item at %s does not have a valid SID", identifierWithSlash);
            return;    
        }
        long firstSID = json_integer_value(firstSIDJSON);
        // clean up if necessary any memory allocated
        json_decref(firstSIDJSON);


        // Set both the maps
        hashmap_set(sidModel->sidIdentifierHashMap,
                    &(SIDIdentifierT){.sid = firstSID, .identifier = (char *)identifierWithSlash});

        hashmap_set(sidModel->identifierSIDHashMap,
                    &(IdentifierSIDT){.sid = firstSID, .identifier = (char *)identifierWithSlash});
    }
    // Clean up if necessary any memory allocated
    json_decref(firstItemMap);


    // Iterate through the "items" containing JSON Maps
    size_t itemsSize = json_array_size(itemsJSON);
    for (size_t i = 0; i < itemsSize; i++) {
        json_t *itemMap = json_array_get(itemsJSON, i);
        if (json_is_object(itemMap)) {
        
            json_t *sidJSON = json_object_get(itemMap, "sid");
            // convert SID JSON to long
            if (!json_is_integer(sidJSON)) {
                fprintf(stderr, "Following item at %zu does not have a valid SID", i);
                continue;
            }
            long sid = json_integer_value(sidJSON);

            json_t *identifierJSON = json_object_get(itemMap, "identifier");
            // convert identifier to char *
            if (!json_is_string(identifierJSON)) {
                fprintf(stderr, "Following item at %zu does not have a valid identifier", i);
                continue;
            }
            const char *identifier = json_string_value(identifierJSON);

            // HACK replace all instances of ":" by "/" in identifier
            char *itemIdentifierWithSlash = malloc((strlen(identifier) + 1) * sizeof(char));
            strcpy(itemIdentifierWithSlash, identifier);    
            char *colon = strchr(itemIdentifierWithSlash, ':');
            while (colon != NULL) {
                *colon = '/';
                colon = strchr(colon, ':');
            }
            
            
            // Set both the maps
            hashmap_set(sidModel->sidIdentifierHashMap,
                        &(SIDIdentifierT){.sid = sid, .identifier = (char *)itemIdentifierWithSlash});

            hashmap_set(sidModel->identifierSIDHashMap,
                        &(IdentifierSIDT){.sid = sid, .identifier = (char *)itemIdentifierWithSlash});

            // typeJSON can be optional
            json_t *typeJSON = json_object_get(itemMap, "type");

            // TODO Handle list types
            if (typeJSON != NULL && json_is_string(typeJSON)) {
                const char *typeString = json_string_value(typeJSON);
                enum SchemaIdentifierTypeEnum type;
                // Convert typeJSON to one of the enum
                if (!strcmp(typeString, "boolean"))
                    type = BOOLEAN;
                else if (!strcmp(typeString, "binary"))
                    type = BOOLEAN;
                else if (!strcmp(typeString, "string"))
                    type = STRING;
                else if (!strcmp(typeString, "decimal64"))
                    type = DECIMAL64;
                else if (!strcmp(typeString, "uint8"))
                    type = UINT_8;
                else if (!strcmp(typeString, "uint16"))
                    type = UINT_16;
                else if (!strcmp(typeString, "uint32"))
                    type = UINT_32;
                else if (!strcmp(typeString, "uint64"))
                    type = UINT_64;
                else if (!strcmp(typeString, "rcs-algorithm-type"))
                    type = RCS_ALGORITHM;
                else if (!strcmp(typeString, "identityref"))
                    type = IDENTITY_REF;
                else {
                    fprintf(stderr, "Unknown Identifier type, assigning type as STRING: %s\n", typeString);
                    type = STRING;
                    // continue;
                }
                hashmap_set(sidModel->identifierTypeHashMap,
                            &(IdentifierTypeT){.identifier = (char *)itemIdentifierWithSlash, .type = type});
            }
        }
    }
}

/*
Convert char* to int64_t, return INTMAX_MIN in case of an error
*/
int64_t char2int64(char *keyString) {
    // Convert char* to int64_t using strtoimax
    intmax_t intValue = strtoimax(keyString, NULL, 10);
    if (intValue == INTMAX_MIN || intValue == INTMAX_MAX) {
        fprintf(stderr, "Conversion error or out of range");
        return INTMAX_MIN;
    }

    // Check for valid conversion
    if (errno == ERANGE) {
        fprintf(stderr, "Value out of range");
        return INTMAX_MIN;
    }

    // Convert intmax_t to int64_t
    int64_t int64Value = (int64_t)intValue;

    return int64Value;
}

char *int2str(char *keyString, int64_t keyInt64) {
    int keyStringLength = snprintf(NULL, 0, "%" PRIu64, keyInt64);
    if (keyStringLength < 0) {
        fprintf(stderr, "snprintf error");
        return NULL;
    }

    // Allocate memory for the char* string
    keyString = (char *)malloc((keyStringLength + 1) * sizeof(char));
    if (keyString == NULL) {
        fprintf(stderr, "Memory allocation failed");
        return NULL;
    }

    // Convert uint64_t to char*
    snprintf(keyString, keyStringLength + 1, "%" PRIu64, keyInt64);
    return keyString;
}

void removeTrailingSlashFromPath(const char *qualifiedPath, char *formattedPath) {
    /*
    Remove the trailing '/' character, assumes formattedPath is initialized properly
    */
    size_t len = strlen(qualifiedPath);

    if (qualifiedPath[len - 1] == '/') {

        // Copy all characters except the last one to the new string.
        strncpy(formattedPath, qualifiedPath, len - 1);
        formattedPath[len - 1] = '\0'; // Add null-terminator.

    } else {
        fprintf(stderr, "Wrong usage of formatPath function");
        strcpy(formattedPath, qualifiedPath);
    }
}

// Used it in coreconf to find the SID of a given identifier
char *getSubstringAfterLastColon(const char *input) {
    char *lastColon = strrchr(input, ':');

    if (lastColon != NULL) {
        // Return the substring after the last ':'
        return lastColon + 1;
    } else {
        // If ':' is not found, return the original string
        return (char *)input;
    }
}
