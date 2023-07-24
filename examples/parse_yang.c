#include <stdio.h>
#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>
#include "hashmap.h"
#include "sid.h"

/*
Allot all memory explicitly in the main
*/
void buildKeyMappingHashMap(struct hashmap *keyMappingHashMap, json_t* root){

    const char* keyMappingString = "key-mapping";
    json_t* keyMappingJSON = json_object_get(root, keyMappingString);
    
    // Check if key-mapping exists in the SID file
    if (!json_is_object(keyMappingJSON)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return;
    }

    // Iterate over the key-mapping tree to build our own datastructure
    const char* key;
    json_t* value;
    json_object_foreach(keyMappingJSON, key, value){
        // Convert key to a long value first
        
        // Create an initialize an an empty list 
        DynamicLongListT *dynamicLongList = malloc(sizeof(DynamicLongListT));
        if (dynamicLongList == NULL){
            fprintf(stderr, "Failed allocating memory to dynamicLongList for the key %s", key);
            continue;
        }
        initializeDynamicLongList(dynamicLongList);

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
            json_t* childSID = json_array_get(value, i);
            if (!json_is_integer(childSID)){
                fprintf(stderr, "Following value is not a valid SID %s ", json_string_value(childSID));
            }
            // Get the long value
            long childSIDLong = json_integer_value(childSID);
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

void printKeyMappingHashMap(struct hashmap *keyMappingHashMap){
    size_t iter = 0;
    void *item;
    while(hashmap_iter(keyMappingHashMap, &iter, &item)){
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }
}


/*Example to read SID file and find a SID and its corresponding value*/
void main(){

    long fileSize;
    const char* jsonFilePath = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/sid_examples/ietf-schc@2022-12-19.sid";
    const char* keyMappingString = "key-mapping";
    struct hashmap *keyMappingHashMap;

    keyMappingHashMap = hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);


    FILE *fp = fopen(jsonFilePath, "r");
    if (!fp){
        fprintf(stderr, "Failed to open the file: %s\n", jsonFilePath);
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer
    char* sidFileBuffer = (char*) malloc(fileSize + 1);
    if (!sidFileBuffer){
        fprintf(stderr, "Failed allotting memory for the buffer.\n");
        fclose(fp);
        return;
    }

    // Read the entire file into the buffer
    size_t bytesRead = fread(sidFileBuffer, 1, fileSize, fp);
    if (!bytesRead){
        fprintf(stderr, "Failed to read the file: %s\n", jsonFilePath);
        fclose(fp);
        return;
    } else {
        printf("File content read %zu", bytesRead);
    }

    // Parse the JSON File
    sidFileBuffer[fileSize] = '\0';
    json_error_t error;
    json_t* root = json_loads(sidFileBuffer, 0, &error);
    if (!root){
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        fclose(fp);
        return;
    }

    // Access key-mapping
    json_t* keyMappingJSON = json_object_get(root, keyMappingString);
    if (!json_is_object(keyMappingJSON)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        fclose(fp);
        return;
    }

    /*
    // Iterate over the json
    const char* key;
    json_t* value;
    json_object_foreach(keyMappingJSON, key, value){
        if (!json_is_array(value)){
            printf("Key %s and the value %s \n", key, json_string_value(value));
        } else {
            printf("Found an array, printing it: \n");
            for (int i=0; i < json_array_size(value); i++){
                json_t *data = json_array_get(value, i);
                if (json_is_number(data)){
                    //printf("For key %s print %lu\n",key, (long) json_number_value(data));  
                }

                }
        }
    }
    */

    // Create and populate an internal hashmap
    // BUILD
    buildKeyMappingHashMap(keyMappingHashMap, root);
    // ITERATE
    size_t iter=0;
    void* item;
    while (hashmap_iter(keyMappingHashMap, &iter, &item)){
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }

    // Cleanup
    hashmap_free(keyMappingHashMap);
    json_decref(root);
    free(sidFileBuffer);
}
