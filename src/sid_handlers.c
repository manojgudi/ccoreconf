#include "sid_handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include ccoreconf headers for hashmap
#include "hashmap.h"
#include "coreconfTypes.h"
#include "sid.h"

// Global handler registry (hashmap of SIDHandlerEntryT keyed by SID)
static struct hashmap *handlerRegistry = NULL;

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

/**
 * Free function for SIDHandlerEntryT
 * Called when hashmap entries are freed
 */
static void SIDHandlerFree(void *item) {
    // SIDHandlerEntryT doesn't allocate any internal memory
    // The identifier and type strings are owned by the caller (typically const strings)
    // So nothing to free
    (void)item;
}

int initializeSIDHandlerRegistry(void) {
    if (handlerRegistry != NULL) {
        fprintf(stderr, "Handler registry already initialized\n");
        return -1;
    }

    handlerRegistry = hashmap_new(sizeof(SIDHandlerEntryT), 0, 0, 0,
                                   SIDHandlerHash, SIDHandlerCompare,
                                   SIDHandlerFree, NULL);

    if (handlerRegistry == NULL) {
        fprintf(stderr, "Failed to create handler registry\n");
        return -1;
    }

    printf("SID handler registry initialized\n");
    return 0;
}

int registerSIDHandler(uint64_t SID,
                       SIDReadHandler readHandler,
                       SIDWriteHandler writeHandler,
                       const char *identifier,
                       const char *type) {
    if (handlerRegistry == NULL) {
        fprintf(stderr, "Handler registry not initialized\n");
        return -1;
    }

    // At least one handler must be provided
    if (readHandler == NULL && writeHandler == NULL) {
        fprintf(stderr, "At least one handler (read or write) must be provided for SID %lu\n", SID);
        return -1;
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
    const SIDHandlerEntryT *existing = hashmap_set(handlerRegistry, &entry);

    if (existing != NULL) {
        printf("Warning: Replacing existing handler for SID %lu\n", SID);
    }

    printf("Registered handler for SID %lu (%s)\n", SID, identifier ? identifier : "unknown");
    return 0;
}

SIDHandlerEntryT* lookupSIDHandler(uint64_t SID) {
    if (handlerRegistry == NULL) {
        return NULL;
    }

    // Create a temporary entry for lookup
    SIDHandlerEntryT lookup = { .SID = SID };
    return (SIDHandlerEntryT*)hashmap_get(handlerRegistry, &lookup);
}

void freeSIDHandlerRegistry(void) {
    if (handlerRegistry != NULL) {
        hashmap_free(handlerRegistry);
        handlerRegistry = NULL;
        printf("SID handler registry freed\n");
    }
}

size_t getHandlerCount(void) {
    if (handlerRegistry == NULL) {
        return 0;
    }
    return hashmap_count(handlerRegistry);
}

void printHandlerRegistry(void) {
    if (handlerRegistry == NULL) {
        printf("Handler registry not initialized\n");
        return;
    }

    size_t count = hashmap_count(handlerRegistry);
    printf("\n=== SID Handler Registry (%zu handlers) ===\n", count);

    if (count == 0) {
        printf("No handlers registered\n");
        return;
    }

    size_t iter = 0;
    void *item;
    while (hashmap_iter(handlerRegistry, &iter, &item)) {
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

        printf("  SID %lu: %s [%s]\n",
               entry->SID,
               entry->identifier ? entry->identifier : "unknown",
               cap_str);
        printf("    Type: %s\n", entry->type ? entry->type : "unknown");
        printf("    Read handler: %s\n", entry->readHandler ? "YES" : "NO");
        printf("    Write handler: %s\n", entry->writeHandler ? "YES" : "NO");
    }

    printf("===================================\n\n");
}
