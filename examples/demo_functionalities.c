#include "../include/ccoreconf.h"
#include "../include/fileOperations.h"
#include "../include/hashmap.h"
#include <jansson.h>
#include <libyang/libyang.h>
#include <stdio.h>
#include <stdlib.h>

/*Example to read SID file and find a SID and its corresponding value*/
void main() {

    // const char* sidFilePath1 =
    // "/home/valentina/projects/lpwan_examples/ccoreconf/samples/sid_examples/ietf-schc@2022-12-19.sid";
    //  sidFile and corresponding config file
    // const char* sidFilePath1 =
    // "/home/valentina/projects/lpwan_examples/ccoreconf/samples/basic/example-1@unknown.sid";
    // const char* configFile1 =
    // "/home/valentina/projects/lpwan_examples/ccoreconf/samples/basic/ex1-config.json";
    const char *sidFilePath1 =
        "./model_sid_files/ietf-schc@2022-12-19.sid";
    const char *sidFilePath2 =
        "./model_sid_files/ietf-schc-oam@2021-11-10.sid";

    // const char *configFile1 = "/home/valentina/projects/lpwan_examples/"
    //"ccoreconf/samples/libconf/ex2-config.json";

    const char *configFile2 = "./model_sid_files/model.json";

    const char *keyMappingString = "key-mapping";
    SIDModelT *sidModel = malloc(sizeof(SIDModelT));
    // struct hashmap *keyMappingHashMap, *identifierSIDHashMap,
    // *sidIdentifierHashMap, *identifierTypeHashMap;

    json_t *sidFile1JSON = readJSON(sidFilePath1);
    json_t *sidFile2JSON = readJSON(sidFilePath2);
    json_t *configFileJSON = readJSON(configFile2);

    // Access key-mapping
    json_t *keyMappingJSON = json_object_get(sidFile1JSON, keyMappingString);
    if (!json_is_object(keyMappingJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return;
    }

    // Build keyMappingHashMap
    sidModel->keyMappingHashMap =
        hashmap_new(sizeof(KeyMappingT), 0, 0, 0, keyMappingHash, keyMappingCompare, NULL, NULL);
    buildKeyMappingHashMap(sidModel->keyMappingHashMap, sidFile1JSON);

    // Build identifierSIDHashMap & sidIdentifierHashMap
    sidModel->identifierSIDHashMap =
        hashmap_new(sizeof(IdentifierSIDT), 0, 0, 0, identifierSIDHash, identifierSIDCompare, NULL, NULL);
    sidModel->sidIdentifierHashMap =
        hashmap_new(sizeof(SIDIdentifierT), 0, 0, 0, sidIdentifierHash, sidIdentifierCompare, NULL, NULL);

    // Build identifierTypeHashMap
    sidModel->identifierTypeHashMap =
        hashmap_new(sizeof(IdentifierTypeT), 0, 0, 0, identifierTypeHash, identifierTypeCompare, NULL, NULL);

    buildSIDModel(sidModel, sidFile1JSON);
    buildSIDModel(sidModel, sidFile2JSON);

    // Build CORECONF of the config file
    lookupSID(configFileJSON, sidModel);

    printf("Finalment: \n");
    // print_json_object(configFileJSON);
    printf("---------\n");

    /* Dump the CORECONF representation into a JSON File
    // Open a file for writing
    FILE *file = fopen("output.json", "w");
    if (!file) {
        fprintf(stderr, "Error opening file for writing\n");
        return 1;
    }

    // Dump the JSON object into the file
    int ret = json_dumpf(configFileJSON, file, JSON_INDENT(4));

    if (ret != 0) {
        fprintf(stderr, "Error dumping JSON to file\n");
        fclose(file);
        json_decref(configFileJSON);
        return 1;
    }
    */

    // Test traverseCORECONF
    /*
        json_t *traversedJSON = json_object();
        traversedJSON = traverseCORECONF(configFileJSON, sidModel, 1000096);
        printf("Traversed: \n");
        print_json_object(traversedJSON);
        printf("---------\n");
    */

   // TODO show functionalities
   // Build coreconf
   // Fetching coreconf models
   // traversing with keys

    // keys MUST be initialized properly and must be NON Empty
    int64_t keys[] = {5, 3, 1000068, 1, 1000018, 0};
    size_t keyLength = sizeof(keys) / sizeof(keys[0]);
    // Build a valid SidIdentifierT object and then call traverseCORECONFWithKeys
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = INT64_MIN;
    sidIdentifier->identifier = "/ietf-schc:schc/rule/entry/target-value/value";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(configFileJSON, sidModel, sidIdentifier, keys, keyLength);

    printf("OUTPUT ");
    print_json_object(traversedJSON_);

    // Cleanup
    free(sidIdentifier);

    // Cleanup
    hashmap_free(sidModel->keyMappingHashMap);
    hashmap_free(sidModel->identifierSIDHashMap);
    hashmap_free(sidModel->sidIdentifierHashMap);
    hashmap_free(sidModel->identifierTypeHashMap);

    json_decref(sidFile1JSON);
    json_decref(configFileJSON);
    // json_decref(traversedJSON);
    json_decref(traversedJSON_);
    free(sidModel);
}
