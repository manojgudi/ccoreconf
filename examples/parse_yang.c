#include <stdio.h>
#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>
#include "hashmap.h"
#include "sid.h"

/*Example to read SID file and find a SID and its corresponding value*/
void main(){

    long fileSize;
    const char* sidFilePath1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/sid_examples/ietf-schc@2022-12-19.sid";
    const char* keyMappingString = "key-mapping";
    SIDModelT *sidModel = malloc(sizeof(SIDModelT));
    //struct hashmap *keyMappingHashMap, *identifierSIDHashMap, *sidIdentifierHashMap, *identifierTypeHashMap;


    FILE *fp = fopen(sidFilePath1, "r");
    if (!fp){
        fprintf(stderr, "Failed to open the file: %s\n", sidFilePath1);
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
        fprintf(stderr, "Failed to read the file: %s\n", sidFilePath1);
        fclose(fp);
        return;
    } else {
        printf("File content read %zu", bytesRead);
    }

    // Parse the JSON File
    sidFileBuffer[fileSize] = '\0';
    json_error_t error;
    json_t* sidFileJSON = json_loads(sidFileBuffer, 0, &error);
    if (!sidFileJSON){
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        fclose(fp);
        return;
    }

    // Access key-mapping
    json_t* keyMappingJSON = json_object_get(sidFileJSON, keyMappingString);
    if (!json_is_object(keyMappingJSON)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        fclose(fp);
        return;
    }
    
    // Build keyMappingHashMap
    sidModel->keyMappingHashMap = hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
    buildKeyMappingHashMap(sidModel->keyMappingHashMap, sidFileJSON);
    // ITERATE
    size_t iter=0;
    void* item;
    while (hashmap_iter(sidModel->keyMappingHashMap, &iter, &item)){
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }

    // Build identifierSIDHashMap & sidIdentifierHashMap
    sidModel->identifierSIDHashMap = hashmap_new(sizeof(IdentifierSIDT), 0,0,0, identifierSIDHash, identifierSIDCompare, NULL, NULL);
    sidModel->sidIdentifierHashMap = hashmap_new(sizeof(SIDIdentifierT), 0,0,0, sidIdentifierHash, sidIdentifierCompare, NULL, NULL);

    // Build identifierTypeHashMap
    sidModel->identifierTypeHashMap = hashmap_new(sizeof(IdentifierTypeT), 0,0,0, identifierTypeHash, identifierTypeCompare, NULL, NULL);

    buildSIDModel(sidModel, sidFileJSON);
    printHashMap(sidModel->sidIdentifierHashMap, SID_IDENTIFIER);
    printHashMap(sidModel->identifierSIDHashMap, IDENTIFIER_SID);
    printHashMap(sidModel->identifierTypeHashMap, IDENTIFIER_TYPE);

    // Cleanup
    hashmap_free(sidModel->keyMappingHashMap);
    hashmap_free(sidModel->identifierSIDHashMap);
    hashmap_free(sidModel->sidIdentifierHashMap);
    hashmap_free(sidModel->identifierTypeHashMap);

    json_decref(sidFileJSON);
    free(sidFileBuffer);
    free(sidModel);
}
