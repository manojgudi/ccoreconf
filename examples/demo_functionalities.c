#include <jansson.h>
#include <nanocbor/nanocbor.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/coreconfManipulation.h"
#include "../include/fileOperations.h"
#include "../include/hashmap.h"
#include "../include/serialization.h"

#define MAX_CBOR_BUFFER_SIZE 8096

/*Example to read SID file and find a SID and its corresponding value*/
int main() {
    /*
    Supply all the SID files and the model files and read the json files
    */
    const char *sidFilePath1 = "./model_sid_files/ietf-schc@2022-12-19.sid";
    const char *sidFilePath2 = "./model_sid_files/ietf-schc-oam@2021-11-10.sid";

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

    // Convert it to keyMappingHashMap to a CBOR Map
    uint8_t keyMappingHashCBOR[MAX_CBOR_BUFFER_SIZE];
    nanocbor_encoder_t keyMappingEncoder;
    nanocbor_encoder_init(&keyMappingEncoder, keyMappingHashCBOR, MAX_CBOR_BUFFER_SIZE);
    keyMappingHashMapToCBOR(sidModel->keyMappingHashMap, &keyMappingEncoder);
    printf("Key Mapping CBOR:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&keyMappingEncoder); i++) {
        printf("%02x ", keyMappingHashCBOR[i]);
    }
    printf("\n");

    // Rebuild the keyMappingHashMap from the CBOR
    nanocbor_value_t keyMappingDecoder;
    nanocbor_decoder_init(&keyMappingDecoder, keyMappingHashCBOR, MAX_CBOR_BUFFER_SIZE);
    struct hashmap *keyMappingHashMapDeserialized = cborToKeyMappingHashMap(&keyMappingDecoder);
    printf("Deserialized Key Mapping: \n");
    printKeyMappingHashMap(keyMappingHashMapDeserialized);

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
    // lookupSID(coreconfModel, sidModel);

    // build coreconfValue from the coreconfModel
    CoreconfValueT *coreconfValue_ = buildCoreconfModelFromJson(coreconfModel, sidModel, "/", 0);

    // Print the coreconfValue
    printf("=========================\n");
    printf("\n\n\nPrint the CORECONF model\n");
    printCoreconf(coreconfValue_);

    // Encode the JSON object into CBOR format using NanoCBOR
    // Adjust as needed
    uint8_t ccborBuffer1[MAX_CBOR_BUFFER_SIZE];
    uint8_t ccborBuffer2[MAX_CBOR_BUFFER_SIZE];
    uint8_t ccborBufferDeserialized[MAX_CBOR_BUFFER_SIZE];

    // Serialize the traversedJSON_ to a CBOR format
    nanocbor_encoder_t cEncoder1, cEncoder2, cEncoderDeserialized;
    nanocbor_encoder_init(&cEncoder1, ccborBuffer1, MAX_CBOR_BUFFER_SIZE);
    coreconfToCBOR(coreconfValue_, &cEncoder1);
    //  nanocbor_fmt_end_indefinite(&encoder);

    // Print the encoded CBOR data
    printf("Encoded CORECONF data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&cEncoder1); i++) {
        printf("%02x ", ccborBuffer1[i]);
    }
    printf("\n");
    printf("=========================\n");

    // Build back coreconfModel from the buffer
    nanocbor_value_t decoder;
    nanocbor_decoder_init(&decoder, ccborBuffer1, MAX_CBOR_BUFFER_SIZE);
    CoreconfValueT *coreconfValueDeserialized = cborToCoreconfValue(&decoder, 0);
    printf("\nDeserialized Coreconf: \n");
    printCoreconf(coreconfValueDeserialized);
    printf("\n");

    // Reserialize the deserialized coreconfValue
    nanocbor_encoder_init(&cEncoderDeserialized, ccborBufferDeserialized, MAX_CBOR_BUFFER_SIZE);
    coreconfToCBOR(coreconfValueDeserialized, &cEncoderDeserialized);
    printf("Reserialized CORECONF data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&cEncoderDeserialized); i++) {
        printf("%02x ", ccborBufferDeserialized[i]);
    }

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
    // Build the CLookup hashmap from CoreconfModel
    buildCLookupHashmapFromCoreconf(coreconfValue_, clookupHashmap, 0, 0);
    printf("Chump lookup Correct: \n");
    printCLookupHashmap(clookupHashmap);

    // Build the CLookup hashmap
    // buildCLookupHashmap(coreconfModel, clookupHashmap, 0, 0);

    // print the clookup hashmap
    printf("Chump lookup: \n");
    printCLookupHashmap(clookupHashmap);

    // Build inputs for key requirements
    uint64_t requestSID = 1000115;
    DynamicLongListT *requestKeys = malloc(sizeof(DynamicLongListT));
    initializeDynamicLongList(requestKeys);

    addLong(requestKeys, 0);
    addLong(requestKeys, 1000018);
    addLong(requestKeys, 1);
    addLong(requestKeys, 1000057);
    addLong(requestKeys, 3);
    addLong(requestKeys, 5);

    // Find the requirement for the SID
    PathNodeT *pathNodes = findRequirementForSID(requestSID, clookupHashmap, sidModel->keyMappingHashMap);

    // Print the PathNodeT
    printf("PathNodeT: \n");
    printPathNode(pathNodes);
    printf("---------\n");

    // Examine the coreconf model value
    CoreconfValueT *examinedValue_ = examineCoreconfValue(coreconfValue_, requestKeys, pathNodes);
    printf("Examined the Coreconf Value subtree: \n");
    printCoreconf(examinedValue_);
    printf("---------\n");

    // Convert it into CBOR
    nanocbor_encoder_init(&cEncoder2, ccborBuffer2, MAX_CBOR_BUFFER_SIZE);
    coreconfToCBOR(examinedValue_, &cEncoder2);
    //  nanocbor_fmt_end_indefinite(&encoder);

    // Print the encoded CBOR data
    printf("Encoded CORECONF data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&cEncoder2); i++) {
        printf("%02x ", ccborBuffer2[i]);
    }
    printf("\n");
    printf("=========================\n");

    // Fix Dynamic Longlist
    freeDynamicLongList(requestKeys);
    freePathNode(pathNodes);

    /*
    //  Legacy call with json_t
    // Examine the coreconf model
    json_t *examinedValue = examineCoreconf(coreconfModel, requestKeys, pathNodes);
    printf("Examined the subtree: \n");
    print_json_object(examinedValue);
    printf("---------\n");
    */

    /*
    Legacy function call with json_t and WITHOUT Chump Lookup
    // Find the nodes corresponding to SID 1000096
    json_t *traversedJSON = traverseCORECONF(coreconfModel, 1000096);
    printf("Obtained the subtree: \n");
    print_json_object(traversedJSON);
    printf("---------\n");
    */

    /*
    Legacy function calls with json_t and WITHOUT Chump lookup
    // Find the nodes corresponding to a String Characteristics and specific keys
    // keys MUST be initialized properly and must be NON Empty
    int keys[] = {5, 3, 1000068, 1, 1000018, 0};
    size_t keyLength = sizeof(keys) / sizeof(keys[0]);

    // Build a valid SidIdentifierT object and then call traverseCORECONFWithKeys
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = 1000113;    // INT64_MIN; // Check for 1000115
    sidIdentifier->identifier = "";  // ietf-schc:schc/rule/entry/target-value";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);

    printf("Obtained the subtree: \n");
    print_json_object(traversedJSON_);
    printf("---------\n");
    // Cleanup
    free(sidIdentifier);
    json_decref(traversedJSON_);
    */

    /* Legacy function calls to print CBOR Data from json_t
    // Encode the JSON object into CBOR format using NanoCBOR
    size_t cbor_buffer_size = 5024;  // Adjust as needed
    uint8_t cbor_buffer[cbor_buffer_size];

    // Serialize the traversedJSON_ to a CBOR format
    nanocbor_encoder_t encoder;
    nanocbor_encoder_init(&encoder, cbor_buffer, cbor_buffer_size);
    json_to_cbor(examinedValue, &encoder);

    // Print the encoded CBOR data
    printf("Encoded CBOR data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&encoder); i++) {
        printf("%02x ", cbor_buffer[i]);
    }
    printf("\n");
    */

    // Cleanup
    hashmap_free(clookupHashmap);
    hashmap_free(sidModel->keyMappingHashMap);
    hashmap_free(sidModel->identifierSIDHashMap);
    hashmap_free(sidModel->sidIdentifierHashMap);
    hashmap_free(sidModel->identifierTypeHashMap);

    json_decref(sidFile1JSON);
    json_decref(coreconfModel);
    // json_decref(traversedJSON);

    free(sidModel);

    return 0;
}
