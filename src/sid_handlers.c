#include "sid_handlers.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include ccoreconf headers for hashmap
#include "hashmap.h"
#include "coreconfModel.h"

/**
 * Hash function for SIDHandlerEntryT based on SID
 */
static uint64_t SIDHandlerHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SIDHandlerEntryT *entry = (const SIDHandlerEntryT *)item;
    return hashmap_murmur(&entry->SID, sizeof(uint64_t), seed0, seed1);
}

/**
 * Compare function for SIDHandlerEntryT based on SID
 * Returns 0 if equal, non-zero if different
 */
static int SIDHandlerCompare(const void *a, const void *b, void *udata) {
    (void)udata;  // Unused for compatibility
    const SIDHandlerEntryT *entry1 = (const SIDHandlerEntryT *)a;
    const SIDHandlerEntryT *entry2 = (const SIDHandlerEntryT *)b;
    return (entry1->SID != entry2->SID);
}

int ccoreconfModelRegisterHandler(CoreconfModelT *model,
                                  uint64_t SID,
                                  SIDReadHandler readHandler,
                                  SIDWriteHandler writeHandler,
                                  const char *identifier,
                                  const char *type) {
    if (model == NULL) {
        fprintf(stderr, "ccoreconfModelRegisterHandler: model is NULL\n");
        return -1;
    }

    // At least one handler must be provided
    if (readHandler == NULL && writeHandler == NULL) {
        fprintf(stderr, "At least one handler (read or write) must be provided for SID %" PRIu64 "\n", SID);
        return -1;
    }

    // Entries own no memory (identifier and type are borrowed), so no free callback
    if (model->handlerHashmap == NULL) {
        model->handlerHashmap = hashmap_new(sizeof(SIDHandlerEntryT), 0, 0, 0,
                                            SIDHandlerHash, SIDHandlerCompare, NULL, NULL);
        if (model->handlerHashmap == NULL) {
            fprintf(stderr, "Failed to create handler registry\n");
            return -1;
        }
    }

    // Determine capability based on which handlers are provided
    SIDHandlerCapability capability;
    if (readHandler != NULL && writeHandler != NULL) {
        capability = SID_HANDLER_READWRITE;
    } else if (readHandler != NULL) {
        capability = SID_HANDLER_READ;
    } else {
        capability = SID_HANDLER_WRITE;
    }

    // Create handler entry
    SIDHandlerEntryT entry = {
        .SID = SID,
        .capability = capability,
        .readHandler = readHandler,
        .writeHandler = writeHandler,
        .identifier = identifier,
        .type = type
    };

    // Add to registry (hashmap_set will copy the entry)
    const SIDHandlerEntryT *existing = hashmap_set(model->handlerHashmap, &entry);
    if (existing == NULL && hashmap_oom(model->handlerHashmap)) {
        fprintf(stderr, "Out of memory registering handler for SID %" PRIu64 "\n", SID);
        return -1;
    }

    if (existing != NULL) {
        printf("Warning: Replacing existing handler for SID %" PRIu64 "\n", SID);
    }

    printf("Registered handler for SID %" PRIu64 " (%s)\n", SID, identifier ? identifier : "unknown");
    return 0;
}

const SIDHandlerEntryT *ccoreconfModelLookupHandler(CoreconfModelT *model, uint64_t SID) {
    if (model == NULL || model->handlerHashmap == NULL) {
        return NULL;
    }

    // Create a temporary entry for lookup
    SIDHandlerEntryT lookup = { .SID = SID };
    return (const SIDHandlerEntryT *)hashmap_get(model->handlerHashmap, &lookup);
}

size_t ccoreconfModelHandlerCount(CoreconfModelT *model) {
    if (model == NULL || model->handlerHashmap == NULL) {
        return 0;
    }
    return hashmap_count(model->handlerHashmap);
}

void ccoreconfModelPrintHandlers(CoreconfModelT *model) {
    size_t count = ccoreconfModelHandlerCount(model);
    printf("\n=== SID Handler Registry (%zu handlers) ===\n", count);

    if (count == 0) {
        printf("No handlers registered\n");
        return;
    }

    size_t iter = 0;
    void *item;
    while (hashmap_iter(model->handlerHashmap, &iter, &item)) {
        const SIDHandlerEntryT *entry = (const SIDHandlerEntryT *)item;

        const char *cap_str;
        switch (entry->capability) {
            case SID_HANDLER_READ:
                cap_str = "READ";
                break;
            case SID_HANDLER_WRITE:
                cap_str = "WRITE";
                break;
            case SID_HANDLER_READWRITE:
                cap_str = "READ/WRITE";
                break;
            default:
                cap_str = "UNKNOWN";
                break;
        }

        printf("  SID %" PRIu64 ": %s [%s]\n",
               entry->SID,
               entry->identifier ? entry->identifier : "unknown",
               cap_str);
        printf("    Type: %s\n", entry->type ? entry->type : "unknown");
        printf("    Read handler: %s\n", entry->readHandler ? "YES" : "NO");
        printf("    Write handler: %s\n", entry->writeHandler ? "YES" : "NO");
    }

    printf("===================================\n\n");
}
