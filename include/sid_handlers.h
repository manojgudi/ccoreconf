#ifndef SID_HANDLERS_H
#define SID_HANDLERS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward declarations from ccoreconf
typedef struct CoreconfValue CoreconfValueT;
typedef struct DynamicLongListStruct DynamicLongListT;
typedef struct PathNode PathNodeT;

/**
 * Handler capabilities - indicates what operations a handler supports
 */
typedef enum {
    SID_HANDLER_READ       = 0x01,  // Handler supports read operations
    SID_HANDLER_WRITE      = 0x02,  // Handler supports write operations
    SID_HANDLER_READWRITE  = 0x03   // Handler supports both read and write
} SidHandlerCapability;

/**
 * Context passed to handler functions
 * Contains all information needed to process a SID request
 */
typedef struct {
    uint64_t sid;                       // The requested SID
    DynamicLongListT *keys;             // Keys for list items (NULL if not a list)
    PathNodeT *pathNode;                // Path information for traversal
    CoreconfValueT *coreconfModel;      // Full model for fallback access
} SidHandlerContext;

/**
 * Read handler function signature
 * @param ctx Context containing SID, keys, and model reference
 * @return Pointer to CoreconfValueT with the result, or NULL to fall back to hashmap
 */
typedef CoreconfValueT* (*SidReadHandler)(SidHandlerContext *ctx);

/**
 * Write handler function signature
 * @param ctx Context containing SID, keys, and model reference
 * @param value The value to write
 * @return 0 on success, non-zero error code on failure
 */
typedef int (*SidWriteHandler)(SidHandlerContext *ctx, CoreconfValueT *value);

/**
 * Handler registry entry
 * Stores handler functions and metadata for a specific SID
 */
typedef struct {
    uint64_t sid;                       // The SID this handler is for
    SidHandlerCapability capability;    // What operations this handler supports
    SidReadHandler readHandler;         // Read function pointer (NULL if not supported)
    SidWriteHandler writeHandler;       // Write function pointer (NULL if not supported)
    const char *identifier;             // YANG identifier for debugging (e.g., "/simple-example:data-store/value")
    const char *type;                   // YANG type for debugging (e.g., "uint32")
} SidHandlerEntry;

/**
 * Initialize the SID handler registry
 * Must be called before any other handler registry functions
 * @return 0 on success, non-zero on error
 */
int initializeSidHandlerRegistry(void);

/**
 * Register a handler for a specific SID
 * @param sid The SID to register a handler for
 * @param readHandler Read function pointer (can be NULL if not supported)
 * @param writeHandler Write function pointer (can be NULL if not supported)
 * @param identifier YANG identifier string for debugging
 * @param type YANG type string for debugging
 * @return 0 on success, non-zero on error
 */
int registerSidHandler(uint64_t sid,
                       SidReadHandler readHandler,
                       SidWriteHandler writeHandler,
                       const char *identifier,
                       const char *type);

/**
 * Lookup a handler for a specific SID
 * @param sid The SID to look up
 * @return Pointer to SidHandlerEntry if found, NULL if not found
 */
SidHandlerEntry* lookupSidHandler(uint64_t sid);

/**
 * Free the SID handler registry and all registered handlers
 * Should be called during cleanup
 */
void freeSidHandlerRegistry(void);

/**
 * Get the number of registered handlers
 * @return Number of handlers in the registry
 */
size_t getHandlerCount(void);

/**
 * Print all registered handlers (for debugging)
 */
void printHandlerRegistry(void);

#endif // SID_HANDLERS_H
