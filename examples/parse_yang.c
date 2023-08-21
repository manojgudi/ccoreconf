#include <stdio.h>
#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>
#include "hashmap.h"
#include "fileOperations.h"
#include "ccoreconf.h"

/*Example to read SID file and find a SID and its corresponding value*/
void main(){

    
    //const char* sidFilePath1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/sid_examples/ietf-schc@2022-12-19.sid";
    // sidFile and corresponding config file
    const char* sidFilePath1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/basic/example-1@unknown.sid";
    const char* configFile1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/basic/ex1-config.json";
    //const char* sidFilePath1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/libconf/example-2@unknown.sid";
    //const char* configFile1 = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/libconf/ex2-config.json";
    
    const char* keyMappingString = "key-mapping";
    SIDModelT *sidModel = malloc(sizeof(SIDModelT));
    //struct hashmap *keyMappingHashMap, *identifierSIDHashMap, *sidIdentifierHashMap, *identifierTypeHashMap;

    json_t *sidFileJSON = readJSON(sidFilePath1);
    json_t *configFileJSON = readJSON(configFile1);

    // Access key-mapping
    json_t* keyMappingJSON = json_object_get(sidFileJSON, keyMappingString);
    if (!json_is_object(keyMappingJSON)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
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


    // Build CORECONF of the config file
    lookupSID(configFileJSON, sidModel);  


    // Cleanup
    hashmap_free(sidModel->keyMappingHashMap);
    hashmap_free(sidModel->identifierSIDHashMap);
    hashmap_free(sidModel->sidIdentifierHashMap);
    hashmap_free(sidModel->identifierTypeHashMap);

    json_decref(sidFileJSON);
    json_decref(configFileJSON);
    free(sidModel);
}
