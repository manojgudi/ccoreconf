#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"
#include "sid.h"
#include <string.h>
#include <math.h>
#include <jansson.h>

#define SID_LENGTH 20
// Array of string representations for the enum values
const char* SchemaIdentifierTypeStrings[] = {
    "String",
    "Unsigned Integer 8bit",
    "Unsigned Integer 16bit",
    "Unsigned Integer 32bit",
    "Unsigned Integer 64bit",
    "RCS Algorithm",
    "Decimal 64bit",
    "Boolean"
};

DynamicLongListT *createDynamicLongList(){
    DynamicLongListT *dynamicLongList  = (DynamicLongListT *) malloc(sizeof(DynamicLongListT));
    
    if (dynamicLongList == NULL){
        fprintf(stderr, "Failed malloc'ing Dynamic long list");
        return NULL;
    } 
    dynamicLongList->longList = NULL;
    dynamicLongList->size = 0;
    return dynamicLongList;
}

void initializeDynamicLongList(DynamicLongListT *dynamicLongList){
    // If its NULL, do nothing;
    if (!dynamicLongList)
        return;
    dynamicLongList->longList = malloc(sizeof(long));
}

void addLong(DynamicLongListT *dynamicLongList, long value){
    size_t currentListSize;
    if (dynamicLongList == NULL){
        currentListSize = 0;
    }else{
        currentListSize = dynamicLongList->size;
    }

    dynamicLongList->longList = (long*) realloc(dynamicLongList->longList, (currentListSize+1) * sizeof(long));
    // Check if realloc happened properly, if no, then realloc failed and longList will be NULL
    if (!dynamicLongList->longList){
        fprintf(stderr, "Failed realloc'ing long list");
        return;
    }
    dynamicLongList->size = currentListSize+1;
    dynamicLongList->longList[currentListSize] = value;
}

void freeDynamicLongList(DynamicLongListT *dynamicLongList){
    // If its NULL, then do nothing
    if (!dynamicLongList)
        return;
    free(dynamicLongList->longList);
    free(dynamicLongList);
}

int keyMappingCompare(const void *a, const void *b, void *udata){
    const KeyMappingT *keyMapping1 = (KeyMappingT*) a;
    const KeyMappingT *keyMapping2 = (KeyMappingT*) b;
    // return strcmp(keyMapping1->key, keyMapping2->key);
    return (keyMapping1->key != keyMapping2->key);
}

uint64_t keyMappingHash(const void* item, uint64_t seed0, uint64_t seed1){
    const KeyMappingT *keyMapping = (KeyMappingT *) item;
    //char keyString[SID_LENGTH];
    //sprintf(keyString, "%lu", keyMapping->key);
    // TODO Can you replace log with something else?
    return hashmap_sip(&keyMapping->key, log(keyMapping->key),seed0, seed1);
}

int identifierSIDCompare(const void *a, const void *b, void *udata){
    const IdentifierSIDT *identifierSID1 = (IdentifierSIDT*) a;
    const IdentifierSIDT *identifierSID2 = (IdentifierSIDT*) b;
    return strcmp(identifierSID1->identifier, identifierSID2->identifier);
}

uint64_t identifierSIDHash(const void* item, uint64_t seed0, uint64_t seed1){
    const IdentifierSIDT *identifierSID =  (IdentifierSIDT *) item;
    return hashmap_sip(identifierSID->identifier, strlen(identifierSID->identifier), seed0, seed1);
}

int sidIdentifierCompare(const void *a, const void *b, void *udata){
    const SIDIdentifierT* sidIdentifier1  = (SIDIdentifierT*) a;
    const SIDIdentifierT* sidIdentifier2  = (SIDIdentifierT*) b;
    return (sidIdentifier1->sid != sidIdentifier2->sid );
}

uint64_t sidIdentifierHash(const void* item, uint64_t seed0, uint64_t seed1){
    const SIDIdentifierT *sidIdentifier = (SIDIdentifierT *) item;
    return hashmap_sip(&sidIdentifier->sid, log(sidIdentifier->sid), seed0, seed1);
}

int identifierTypeCompare(const void *a, const void *b, void *udata){
    const IdentifierTypeT* identifierType1 = (IdentifierTypeT*) a;
    const IdentifierTypeT* identifierType2 = (IdentifierTypeT*) b;
    return strcmp(identifierType1->identifier, identifierType2->identifier);
}

uint64_t identifierTypeHash(const void* item, uint64_t seed0, uint64_t seed1){
    const IdentifierTypeT* identifierType = (IdentifierTypeT*) item;
    return hashmap_sip(identifierType->identifier, strlen(identifierType->identifier), seed0, seed1);
}

// NOTE This doesn't work well, idk why, it has to be initialized from the main
void initializeKeyMappingHashMap(struct hashmap *keyMappingHashMap){
    keyMappingHashMap = hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
}


/*
Allot all memory explicitly in the main
*/
void buildKeyMappingHashMap(struct hashmap *keyMappingHashMap, json_t *sidFileJSON){

    const char* keyMappingString = "key-mapping";
    json_t* keyMappingJSON = json_object_get(sidFileJSON, keyMappingString);
    
    // Check if key-mapping exists in the SID file
    if (!json_is_object(keyMappingJSON)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return;
    }

    // Iterate over the key-mapping tree to build our own datastructure
    const char* key;
    json_t* value;
    json_object_foreach(keyMappingJSON, key, value){
        
        // Create an initialize an an empty list 
        DynamicLongListT *dynamicLongList = malloc(sizeof(DynamicLongListT));
        if (dynamicLongList == NULL){
            fprintf(stderr, "Failed allocating memory to dynamicLongList for the key %s", key);
            continue;
        }
        initializeDynamicLongList(dynamicLongList);

        // Convert key to a long value first
        char *endPtr;
        long parentSID = strtol(key, &endPtr, 10);
        if (endPtr == key){
            fprintf(stderr, "Failed converting the key to a long %s", key);
            continue;
        }

        // If the value is not an array then continue without adding that key
        if (!json_is_array(value)){
            fprintf(stderr, "Following key %s doesn't have valid SID children", key);
            continue;
        }

        for (size_t i=0; i < json_array_size(value); i++){
            json_t* childSIDJSON = json_array_get(value, i);
            if (!json_is_integer(childSIDJSON)){
                fprintf(stderr, "Following value is not a valid SID %s ", json_string_value(childSIDJSON));
                continue;
            }
            // Get the long value
            long childSIDLong = json_integer_value(childSIDJSON);
            // Add value to the dynamicLongList
            addLong(dynamicLongList, childSIDLong);
        }
        // Populate the Hashmap
        hashmap_set(keyMappingHashMap, &(KeyMappingT){.key=parentSID, .dynamicLongList=dynamicLongList});
    }

}

void printKeyMappingT(const KeyMappingT *keyMapping){
    printf("\nFor the key %lu: \n", keyMapping->key);

    // Iterate over DynamicLongListT
    for (int i=0; i < keyMapping->dynamicLongList->size; i++){
        long childSID = *(keyMapping->dynamicLongList->longList + i);
        printf("%lu, ", childSID);
    }
}

void printIdentifierSIDT(const IdentifierSIDT *identifierSID){
    printf("\nIdentifier %s: %lu (SID) ", identifierSID->identifier, identifierSID->sid);
}

void printSIDIdentifierT(const SIDIdentifierT *sidIdentifier){
    printf("\nSID %lu: %s (Identifier) ", sidIdentifier->sid, sidIdentifier->identifier);
}

void printIdentifierTypeT(const IdentifierTypeT *identifierType){
    printf("\nIdentifier %s: %s (type) ", identifierType->identifier, SchemaIdentifierTypeStrings[identifierType->type]);
}

void printKeyMappingHashMap(struct hashmap *keyMappingHashMap){
    size_t iter = 0;
    void *item;
    while(hashmap_iter(keyMappingHashMap, &iter, &item)){
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }
}

void printHashMap(struct hashmap* anyHashMap, enum HashMapTypeEnum hashmapType){
    size_t iter = 0;
    void *item;
    while(hashmap_iter(anyHashMap, &iter, &item)){
        switch(hashmapType){
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



void buildSIDModel(SIDModelT *sidModel, json_t *sidFileJSON){
    const char* itemsString = "items";
    json_t* itemsJSON = json_object_get(sidFileJSON, itemsString);

    // Check if itemsString exists in the SID file and if its correctly parsed
    if (!json_is_array(itemsJSON)){
        fprintf(stderr, "Failed %s does not return a JSON array:", itemsString);
        return;
    }

    // Iterate through the "items" containing JSON Maps
    size_t itemsSize = json_array_size(itemsJSON);
    for (size_t i=0; i<itemsSize; i++){
        json_t *itemMap = json_array_get(itemsJSON, i);
        if (json_is_object(itemMap)){
            json_t *sidJSON = json_object_get(itemMap, "sid");
            // convert SID JSON to long
            if(!json_is_integer(sidJSON)){
                fprintf(stderr, "Following item at %zu does not have a valid SID", i);
                continue;
            }
            long sid = json_integer_value(sidJSON);

            json_t *identifierJSON = json_object_get(itemMap, "identifier");
            // convert identifier to char *
            if (!json_is_string(identifierJSON)){
                fprintf(stderr, "Following item at %zu does not have a valid identifier", i);
                continue;
            }
            const char *identifier = json_string_value(identifierJSON);

            // Set both the maps
            hashmap_set(sidModel->sidIdentifierHashMap, &(SIDIdentifierT){.sid=sid, .identifier=(char *)identifier});

            hashmap_set(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.sid=sid, .identifier=(char *)identifier});

            // typeJSON can be optional
            json_t *typeJSON = json_object_get(itemMap, "type");
            
            // TODO Handle list types
            if (typeJSON != NULL && json_is_string(typeJSON)){
                const char* typeString = json_string_value(typeJSON);
                enum SchemaIdentifierTypeEnum type;
                // Convert typeJSON to one of the enum
                if (!strcmp(typeString, "boolean"))
                    type = BOOLEAN;
                else if(!strcmp(typeString, "string"))
                    type = STRING;
                else if (!strcmp(typeString, "decimal64"))
                    type = DECIMAL64;
                else if (!strcmp(typeString, "uint8"))
                    type = UINT_8;
                else if (!strcmp(typeString, "uint16"))
                    type = UINT_16;
                else if (!strcmp(typeString, "rcs-algorithm-type"))
                    type = RCS_ALGORITHM;
                else {
                    fprintf(stderr, "Unknown Identifier type %s", typeString);
                    continue;
                }
                hashmap_set(sidModel->identifierTypeHashMap, &(IdentifierTypeT){.identifier = (char *)identifier, .type=type});
            }
        }
    }

}


void toCoreConf(){

}

void removeTrailingSlashFromPath(const char* qualifiedPath, char *formattedPath) {
    /*
    Remove the trailing '/' character, assumes formattedPath is initialized properly
    */
    size_t len = strlen(qualifiedPath);

    if (qualifiedPath[len-1]=='/'){

    // Copy all characters except the last one to the new string.
    strncpy(formattedPath, qualifiedPath, len - 1);
    formattedPath[len] = '\0'; // Add null-terminator.

    } else {
        fprintf(stderr, "Wrong usage of formatPath function");
        strcpy(formattedPath, qualifiedPath);
    }

}

