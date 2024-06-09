#include <nanocbor/nanocbor.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/coreconfManipulation.h"
#include "../include/coreconfTypes.h"
#include "../include/serialization.h"
#include "coreconf_model_cbor.h"

/*Example to read cbor from coreconfModelCBORBuffer and load it into Coreconf Model*/
int main(void) {
    // Read cbor from coreconfModelCBORBuffer
    nanocbor_value_t decoder;
    nanocbor_decoder_init(&decoder, coreconfModelCBORBuffer, MAX_CBOR_BUFFER_SIZE);

    CoreconfValueT *coreconfModel = cborToCoreconfValue(&decoder, 0);
    printf("\nDeserialized Coreconf: \n");
    printCoreconf(coreconfModel);
    printf("\n");

    // Load key-mapping from keyMappingCBORBuffer
    nanocbor_value_t keyMappingDecoder;
    nanocbor_decoder_init(&keyMappingDecoder, keyMappingCBORBuffer, MAX_CBOR_BUFFER_SIZE);
    struct hashmap *keyMappingHashMap = cborToKeyMappingHashMap(&keyMappingDecoder);

    // Traverse through the deserialized Coreconf
    printf("\nTraversing through the deserialized Coreconf: \n");

    // Build Chump Lookup hashmap for faster lookups
    struct hashmap *clookupHashmap = hashmap_new(sizeof(CLookupT), 0, 0, 0, clookupHash, clookupCompare, NULL, NULL);

    // Build the CLookup hashmap from CoreconfModel
    buildCLookupHashmapFromCoreconf(coreconfModel, clookupHashmap, 0, 0);
    printf("Chump lookup Correct: \n");
    printCLookupHashmap(clookupHashmap);

    // Build inputs for key requirements
    uint64_t requestSID = 1008;
    DynamicLongListT *requestKeys = malloc(sizeof(DynamicLongListT));
    initializeDynamicLongList(requestKeys);

    addLong(requestKeys, 2);

    // Find the requirement for the SID
    PathNodeT *pathNodes = findRequirementForSID(requestSID, clookupHashmap, keyMappingHashMap);

    // Print the PathNodeT
    printf("PathNodeT: \n");
    printPathNode(pathNodes);
    printf("---------\n");

    // Examine the coreconf model value
    CoreconfValueT *examinedValue_ = examineCoreconfValue(coreconfModel, requestKeys, pathNodes);
    printf("Examined the Coreconf Value subtree: \n");
    printCoreconf(examinedValue_);
    printf("---------\n");
    // Free the memory
    freeCoreconf(coreconfModel, true);
    freeCoreconf(examinedValue_, true);
    freeDynamicLongList(requestKeys);
    freePathNode(pathNodes);
    hashmap_free(clookupHashmap);
    hashmap_free(keyMappingHashMap);

    return 0;
}