#ifndef SID_HANDLERS_H
#define SID_HANDLERS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward declarations from ccoreconf
typedef struct CoreconfValue CoreconfValueT;
typedef struct DynamicLongListStruct DynamicLongListT;
typedef struct PathNode PathNodeT;
typedef struct CoreconfModel CoreconfModelT;

/**
 * Handler capabilities - indicates what operations a handler supports
 */
typedef enum {
    SID_HANDLER_READ       = 0x01,  // Handler supports read operations
    SID_HANDLER_WRITE      = 0x02,  // Handler supports write operations
    SID_HANDLER_READWRITE  = 0x03   // Handler supports both read and write
} SIDHandlerCapability;

/**
 * Context passed to handler functions
 * Contains all information needed to process a SID request
 */
typedef struct {
    uint64_t SID;                       // The requested SID
    DynamicLongListT *keys;             // Keys of the lists on the path (NULL if none), in the order of
                                        // ccoreconfModelExamineCoreconfValue: read from the END, outermost
                                        // list first, each list's keys in key-mapping order.  A list-entry
                                        // write takes the entry's own keys from the written value instead.
    PathNodeT *pathNode;                // Path information for traversal
    CoreconfModelT *model;              // Model the SID belongs to (tree, lookups, handlers)
} SIDHandlerContextT;

/**
 * Read handler function signature
 * @param ctx Context containing SID, keys, and model reference
 * @return Pointer to CoreconfValueT with the result, or NULL to fall back to hashmap
 */
typedef CoreconfValueT* (*SIDReadHandler)(SIDHandlerContextT *ctx);

/**
 * Write handler function signature
 *
 * A write REPLACES the node at ctx->SID: a leaf gets the new value, and a
 * container or list entry becomes exactly `value` (children not in `value`
 * are removed).  Writes never merge.
 *
 * @param ctx Context containing SID, keys, and model reference
 * @param value The value to write.  On success (0) the model takes ownership
 *              of it; on failure the caller still owns it.
 * @return 0 on success, non-zero error code on failure
 */
typedef int (*SIDWriteHandler)(SIDHandlerContextT *ctx, CoreconfValueT *value);

/**
 * Handler registry entry
 * Stores handler functions and metadata for a specific SID
 */
typedef struct {
    uint64_t SID;                       // The SID this handler is for
    SIDHandlerCapability capability;    // What operations this handler supports
    SIDReadHandler readHandler;         // Read function pointer (NULL if not supported)
    SIDWriteHandler writeHandler;       // Write function pointer (NULL if not supported)
    const char *identifier;             // YANG identifier for debugging (e.g., "/simple-example:data-store/value")
    const char *type;                   // YANG type for debugging (e.g., "uint32")
} SIDHandlerEntryT;

/**
 * Register a handler for a SID in `model`.  Each model keeps its own
 * registry, so the same SID can have different handlers in different models.
 * Registering a SID twice replaces the earlier handler.
 * `identifier` and `type` are borrowed, not copied: they must outlive the model
 * (string literals from generated code do).
 * @param model Model to register the handler in
 * @param SID The SID to register a handler for
 * @param readHandler Read function pointer (can be NULL if not supported)
 * @param writeHandler Write function pointer (can be NULL if not supported)
 * @param identifier YANG identifier string for debugging
 * @param type YANG type string for debugging
 * @return 0 on success, -1 on error (NULL model, no handlers, out of memory)
 */
int ccoreconfModelRegisterHandler(CoreconfModelT *model,
                                  uint64_t SID,
                                  SIDReadHandler readHandler,
                                  SIDWriteHandler writeHandler,
                                  const char *identifier,
                                  const char *type);

/**
 * Look up the handler registered for `SID` in `model`.
 * @return Pointer to the entry (owned by the model; valid until the next
 *         registration or ccoreconfModelFree), or NULL if none
 */
const SIDHandlerEntryT *ccoreconfModelLookupHandler(CoreconfModelT *model, uint64_t SID);

/**
 * @return Number of handlers registered in `model` (0 for NULL)
 */
size_t ccoreconfModelHandlerCount(CoreconfModelT *model);

/**
 * Print all handlers registered in `model` (for debugging)
 */
void ccoreconfModelPrintHandlers(CoreconfModelT *model);

#endif // SID_HANDLERS_H
