#include "sid_handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include ccoreconf headers for hashmap
#include "hashmap.h"
#include "coreconfTypes.h"
#include "sid.h"

// Global handler registry (hashmap of SidHandlerEntry keyed by SID)
static struct hashmap *handlerRegistry = NULL;

/**
 * Hash function for SidHandlerEntry based on SID
 */
static uint64_t sidHandlerHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SidHandlerEntry *entry = (const SidHandlerEntry *)item;
    return hashmap_murmur(&entry->sid, sizeof(uint64_t), seed0, seed1);
}

/**
 * Compare function for SidHandlerEntry based on SID
 * Returns 0 if equal, non-zero if different
 */
static int sidHandlerCompare(const void *a, const void *b, void *udata) {
    (void)udata;  // Unused for compatibility
    const SidHandlerEntry *entry1 = (const SidHandlerEntry *)a;
    const SidHandlerEntry *entry2 = (const SidHandlerEntry *)b;
    return (entry1->sid != entry2->sid);
}

/**
 * Free function for SidHandlerEntry
 * Called when hashmap entries are freed
 */
static void sidHandlerFree(void *item) {
    // SidHandlerEntry doesn't allocate any internal memory
    // The identifier and type strings are owned by the caller (typically const strings)
    // So nothing to free
    (void)item;
}

int initializeSidHandlerRegistry(void) {
    if (handlerRegistry != NULL) {
        fprintf(stderr, "Handler registry already initialized\n");
        return -1;
    }

    handlerRegistry = hashmap_new(sizeof(SidHandlerEntry), 0, 0, 0,
                                   sidHandlerHash, sidHandlerCompare,
                                   sidHandlerFree, NULL);

    if (handlerRegistry == NULL) {
        fprintf(stderr, "Failed to create handler registry\n");
        return -1;
    }

    printf("SID handler registry initialized\n");
    return 0;
}

int registerSidHandler(uint64_t sid,
                       SidReadHandler readHandler,
                       SidWriteHandler writeHandler,
                       const char *identifier,
                       const char *type) {
    if (handlerRegistry == NULL) {
        fprintf(stderr, "Handler registry not initialized\n");
        return -1;
    }

    // At least one handler must be provided
    if (readHandler == NULL && writeHandler == NULL) {
        fprintf(stderr, "At least one handler (read or write) must be provided for SID %lu\n", sid);
        return -1;
    }

    // Determine capability based on which handlers are provided
    SidHandlerCapability capability;
    if (readHandler != NULL && writeHandler != NULL) {
        capability = SID_HANDLER_READWRITE;
    } else if (readHandler != NULL) {
        capability = SID_HANDLER_READ;
    } else {
        capability = SID_HANDLER_WRITE;
    }

    // Create handler entry
    SidHandlerEntry entry = {
        .sid = sid,
        .capability = capability,
        .readHandler = readHandler,
        .writeHandler = writeHandler,
        .identifier = identifier,
        .type = type
    };

    // Add to registry (hashmap_set will copy the entry)
    const SidHandlerEntry *existing = hashmap_set(handlerRegistry, &entry);

    if (existing != NULL) {
        printf("Warning: Replacing existing handler for SID %lu\n", sid);
    }

    printf("Registered handler for SID %lu (%s)\n", sid, identifier ? identifier : "unknown");
    return 0;
}

SidHandlerEntry* lookupSidHandler(uint64_t sid) {
    if (handlerRegistry == NULL) {
        return NULL;
    }

    // Create a temporary entry for lookup
    SidHandlerEntry lookup = { .sid = sid };
    return (SidHandlerEntry*)hashmap_get(handlerRegistry, &lookup);
}

void freeSidHandlerRegistry(void) {
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
        const SidHandlerEntry *entry = (const SidHandlerEntry *)item;

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
               entry->sid,
               entry->identifier ? entry->identifier : "unknown",
               cap_str);
        printf("    Type: %s\n", entry->type ? entry->type : "unknown");
        printf("    Read handler: %s\n", entry->readHandler ? "YES" : "NO");
        printf("    Write handler: %s\n", entry->writeHandler ? "YES" : "NO");
    }

    printf("===================================\n\n");
}
