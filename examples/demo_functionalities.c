#include "../include/ccoreconf.h"
#include "../include/fileOperations.h"
#include "../include/hashmap.h"
#include "../include/serialization.h"
#include <jansson.h>
#include <libyang/libyang.h>
#include <stdio.h>
#include <stdlib.h>
#include <nanocbor/nanocbor.h>



/*Example to read SID file and find a SID and its corresponding value*/
int main() {

    /*
    Supply all the SID files and the model files and read the json files
    */ 
    const char *sidFilePath1 =
        "./model_sid_files/ietf-schc@2022-12-19.sid";
    const char *sidFilePath2 =
        "./model_sid_files/ietf-schc-oam@2021-11-10.sid";

    const char *configFile2 = "./model_sid_files/model.json";

    const char *keyMappingString = "key-mapping";
    SIDModelT *sidModel = malloc(sizeof(SIDModelT));

    json_t *sidFile1JSON = readJSON(sidFilePath1);
    json_t *sidFile2JSON = readJSON(sidFilePath2);
    json_t *coreconfModel = readJSON(configFile2);

    // Access key-mapping
    json_t *keyMappingJSON = json_object_get(sidFile1JSON, keyMappingString);
    if (!json_is_object(keyMappingJSON)) {
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        return 1;
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

    // Create a clookup hashmap for faster lookups
    struct hashmap *clookupHashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0, clookupHash, clookupCompare, NULL, NULL);

    // Build SIDModel 
    buildSIDModel(sidModel, sidFile1JSON);
    buildSIDModel(sidModel, sidFile2JSON);

    // Build CORECONF representation of the model file
    lookupSID(coreconfModel, sidModel);

    printf("Print the CORECONF model\n");
    //print_json_object(coreconfModel);
    printf("---------\n");

    /* Dump the CORECONF model representation into a JSON format
    // Open a file for writing
    FILE *file = fopen("output.json", "w");
    if (!file) {
        fprintf(stderr, "Error opening file for writing\n");
        return 1;
    }

    // Dump the JSON object into the file
    int ret = json_dumpf(coreconfModel, file, JSON_INDENT(4));

    if (ret != 0) {
        fprintf(stderr, "Error dumping JSON to file\n");
        fclose(file);
        json_decref(coreconfModel);
        return 1;
    }
    */

    // Build the CLookup hashmap
    buildCLookupHashmap(coreconfModel, clookupHashmap, 0, 0);

    // print the clookup hashmap
    printf("Chump lookup: \n");
    printCLookupHashmap(clookupHashmap);
    
 // Build inputs for key requirements
    uint64_t requestSID = 1000115;
    DynamicLongListT *requestKeys = malloc(sizeof(DynamicLongListT));
    initializeDynamicLongList(requestKeys);

    addLong(requestKeys, 1);
    addLong(requestKeys, 1000018);
    addLong(requestKeys, 1);
    addLong(requestKeys, 1000057);
    addLong(requestKeys, 3);
    addLong(requestKeys, 5) ;

    // Find the requirement for the SID
    PathNodeT* pathNodes = findRequirementForSID(requestSID, clookupHashmap, sidModel->keyMappingHashMap);

    // Print the PathNodeT
    printf("PathNodeT: \n");
    printPathNode(pathNodes);
    printf("---------\n");


    // Examine the coreconf model
    json_t *examinedValue = examineCoreconf(coreconfModel, requestKeys, pathNodes);
    printf("Examined the subtree: \n");
    print_json_object(examinedValue);
    printf("---------\n");


    // Fix Dynamic Longlist
    freeDynamicLongList(requestKeys);
    //freeDynamicLongList(emptyList);
    freePathNode(pathNodes);


    /* Find the nodes corresponding to SID 1000096  */
    //json_t *traversedJSON = traverseCORECONF(coreconfModel, 1000096);
    printf("Obtained the subtree: \n");
    //print_json_object(traversedJSON);
    printf("---------\n");

    /*Find the nodes corresponding to a String Characteristics and specific keys*/

    // keys MUST be initialized properly and must be NON Empty
    int keys[] = {5, 3, 1000068, 1, 1000018, 0};
    size_t keyLength = sizeof(keys) / sizeof(keys[0]);
    
    // Build a valid SidIdentifierT object and then call traverseCORECONFWithKeys
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = 1000113 ;//INT64_MIN; // Check for 1000115
    sidIdentifier->identifier = ""; //ietf-schc:schc/rule/entry/target-value";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);

    printf("Obtained the subtree: \n");
    //print_json_object(traversedJSON_);
    printf("---------\n");


    // Encode the JSON object into CBOR format using NanoCBOR
    size_t cbor_buffer_size = 5024; // Adjust as needed
    uint8_t cbor_buffer[cbor_buffer_size];

    // Serialize the traversedJSON_ to a CBOR format
    nanocbor_encoder_t encoder;
    nanocbor_encoder_init(&encoder, cbor_buffer, cbor_buffer_size); 
    json_to_cbor(traversedJSON_, &encoder);
    nanocbor_fmt_end_indefinite(&encoder);

    // Print the encoded CBOR data
    printf("Encoded CBOR data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&encoder); i++) {
        printf("%02x ", cbor_buffer[i]);
    }
    printf("\n");



    // Cleanup
    free(sidIdentifier);

    // Cleanup
    hashmap_free(clookupHashmap);
    hashmap_free(sidModel->keyMappingHashMap);
    hashmap_free(sidModel->identifierSIDHashMap);
    hashmap_free(sidModel->sidIdentifierHashMap);
    hashmap_free(sidModel->identifierTypeHashMap);

    json_decref(sidFile1JSON);
    json_decref(coreconfModel);
    // json_decref(traversedJSON);
    json_decref(traversedJSON_);
    free(sidModel);

    return 0;
}
