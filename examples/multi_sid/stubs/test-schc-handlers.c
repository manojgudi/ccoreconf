#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "test-schc-handlers.h"
#include "test-schc-impl.h"
#include "sid_handlers.h"
#include "coreconfTypes.h"
#include "coreconfManipulation.h"
#include "coreconfModel.h"
#include "dynamicLongList.h"
CoreconfValueT* handler_read_60095(SIDHandlerContextT *ctx) {
    // Call user-implemented read function
    CoreconfValueT* result = read_ietfSchc_schc();

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60095(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // A container is written as a whole: expect a map of its children
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for container write to SID 60095\n");
        return -1;
    }

    // Call user-implemented write function
    int result = write_ietfSchc_schc(value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60095, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60095\n");
            return -1;
        }

        // A write replaces: the container becomes exactly the written value
        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60095\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60096(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60096 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    CoreconfValueT* result = read_schc_rule(rule_ruleIdValue, rule_ruleIdLength);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60096(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value is a hashmap containing keys and data
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for list item write to SID 60096\n");
        return -1;
    }

    // The entry's own keys, from the written value (stored with delta SIDs)
    CoreconfValueT* rule_ruleIdValue_value = getCoreconfHashMap(value->data.map_value, 34);
    if (rule_ruleIdValue_value == NULL) {
        printf("Error: Missing key rule_ruleIdValue (SID 60130) in list item\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)getCoreconfValueAsUint64(rule_ruleIdValue_value);

    CoreconfValueT* rule_ruleIdLength_value = getCoreconfHashMap(value->data.map_value, 33);
    if (rule_ruleIdLength_value == NULL) {
        printf("Error: Missing key rule_ruleIdLength (SID 60129) in list item\n");
        return -1;
    }
    uint64_t rule_ruleIdLength = (uint64_t)getCoreconfValueAsUint64(rule_ruleIdLength_value);

    // Call user-implemented write function
    int result = write_schc_rule(rule_ruleIdValue, rule_ruleIdLength, value);

    // If user function succeeded, update the datastore (list item)
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60096, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for list SID 60096\n");
            return -1;
        }

        // Get array at list SID using delta
        CoreconfValueT* arrayValue = getCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID);
        if (arrayValue == NULL) {
            printf("Error: List array for SID 60096 not found in coreconfModel\n");
            return -1;
        }
        if (arrayValue->type != CORECONF_ARRAY) {
            printf("Error: SID 60096 is not an array in coreconfModel\n");
            return -1;
        }

        // Replace the entry whose keys all match, or append; the array takes value
        static const uint64_t keyDeltas[] = { 34, 33 };
        if (updateCoreconfArrayByKeys(arrayValue, keyDeltas, 2, value) != 0) {
            printf("Error: Failed to update list entry for SID 60096\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60097(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60097 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_ackBehavior(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60097(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60097\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60097 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_ackBehavior(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60097, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60097\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60097\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60098(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60098 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_direction(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60098(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60098\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60098 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_direction(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60098, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60098\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60098\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60099(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60099 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_dtagSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60099(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60099\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60099 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_dtagSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60099, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60099\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60099\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60100(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60100 needs 5 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented read function
    CoreconfValueT* result = read_rule_entry(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60100(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value is a hashmap containing keys and data
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for list item write to SID 60100\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60100 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // The entry's own keys, from the written value (stored with delta SIDs)
    CoreconfValueT* entry_fieldId_value = getCoreconfHashMap(value->data.map_value, 6);
    if (entry_fieldId_value == NULL) {
        printf("Error: Missing key entry_fieldId (SID 60106) in list item\n");
        return -1;
    }
    uint64_t entry_fieldId = (uint64_t)getCoreconfValueAsUint64(entry_fieldId_value);

    CoreconfValueT* entry_fieldPosition_value = getCoreconfHashMap(value->data.map_value, 8);
    if (entry_fieldPosition_value == NULL) {
        printf("Error: Missing key entry_fieldPosition (SID 60108) in list item\n");
        return -1;
    }
    uint64_t entry_fieldPosition = (uint64_t)getCoreconfValueAsUint64(entry_fieldPosition_value);

    CoreconfValueT* entry_directionIndicator_value = getCoreconfHashMap(value->data.map_value, 5);
    if (entry_directionIndicator_value == NULL) {
        printf("Error: Missing key entry_directionIndicator (SID 60105) in list item\n");
        return -1;
    }
    uint64_t entry_directionIndicator = (uint64_t)getCoreconfValueAsUint64(entry_directionIndicator_value);

    // Call user-implemented write function
    int result = write_rule_entry(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, value);

    // If user function succeeded, update the datastore (list item)
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60100, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for list SID 60100\n");
            return -1;
        }

        // Get array at list SID using delta
        CoreconfValueT* arrayValue = getCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID);
        if (arrayValue == NULL) {
            printf("Error: List array for SID 60100 not found in coreconfModel\n");
            return -1;
        }
        if (arrayValue->type != CORECONF_ARRAY) {
            printf("Error: SID 60100 is not an array in coreconfModel\n");
            return -1;
        }

        // Replace the entry whose keys all match, or append; the array takes value
        static const uint64_t keyDeltas[] = { 6, 8, 5 };
        if (updateCoreconfArrayByKeys(arrayValue, keyDeltas, 3, value) != 0) {
            printf("Error: Failed to update list entry for SID 60100\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60101(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60101 needs 5 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented read function
    uint64_t result = read_entry_compDecompAction(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60101(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60101\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60101 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented write function
    int result = write_entry_compDecompAction(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60101, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60101\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60101\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60102(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 6) {
        printf("Error: SID 60102 needs 6 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];
    uint64_t compDecompActionValue_index = (uint64_t)ctx->keys->longList[ctx->keys->size - 6];

    // Call user-implemented read function
    CoreconfValueT* result = read_entry_compDecompActionValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, compDecompActionValue_index);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60102(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value is a hashmap containing keys and data
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for list item write to SID 60102\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60102 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // The entry's own keys, from the written value (stored with delta SIDs)
    CoreconfValueT* compDecompActionValue_index_value = getCoreconfHashMap(value->data.map_value, 1);
    if (compDecompActionValue_index_value == NULL) {
        printf("Error: Missing key compDecompActionValue_index (SID 60103) in list item\n");
        return -1;
    }
    uint64_t compDecompActionValue_index = (uint64_t)getCoreconfValueAsUint64(compDecompActionValue_index_value);

    // Call user-implemented write function
    int result = write_entry_compDecompActionValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, compDecompActionValue_index, value);

    // If user function succeeded, update the datastore (list item)
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60102, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for list SID 60102\n");
            return -1;
        }

        // Get array at list SID using delta
        CoreconfValueT* arrayValue = getCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID);
        if (arrayValue == NULL) {
            printf("Error: List array for SID 60102 not found in coreconfModel\n");
            return -1;
        }
        if (arrayValue->type != CORECONF_ARRAY) {
            printf("Error: SID 60102 is not an array in coreconfModel\n");
            return -1;
        }

        // Replace the entry whose keys all match, or append; the array takes value
        static const uint64_t keyDeltas[] = { 1 };
        if (updateCoreconfArrayByKeys(arrayValue, keyDeltas, 1, value) != 0) {
            printf("Error: Failed to update list entry for SID 60102\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60107(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60107 needs 5 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented read function
    CoreconfValueT* result = read_entry_fieldLength(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60107(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    // Union leaf: any of its member types (identityref, uint8)
    if (!(isTypeUint(value->type))) {
        printf("Error: Expected one of identityref, uint8 for SID 60107\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60107 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented write function
    int result = write_entry_fieldLength(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60107, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60107\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60107\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60109(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60109 needs 5 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented read function
    uint64_t result = read_entry_matchingOperator(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60109(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60109\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60109 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // Call user-implemented write function
    int result = write_entry_matchingOperator(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60109, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60109\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60109\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60110(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 6) {
        printf("Error: SID 60110 needs 6 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];
    uint64_t matchingOperatorValue_index = (uint64_t)ctx->keys->longList[ctx->keys->size - 6];

    // Call user-implemented read function
    CoreconfValueT* result = read_entry_matchingOperatorValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, matchingOperatorValue_index);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60110(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value is a hashmap containing keys and data
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for list item write to SID 60110\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60110 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // The entry's own keys, from the written value (stored with delta SIDs)
    CoreconfValueT* matchingOperatorValue_index_value = getCoreconfHashMap(value->data.map_value, 1);
    if (matchingOperatorValue_index_value == NULL) {
        printf("Error: Missing key matchingOperatorValue_index (SID 60111) in list item\n");
        return -1;
    }
    uint64_t matchingOperatorValue_index = (uint64_t)getCoreconfValueAsUint64(matchingOperatorValue_index_value);

    // Call user-implemented write function
    int result = write_entry_matchingOperatorValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, matchingOperatorValue_index, value);

    // If user function succeeded, update the datastore (list item)
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60110, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for list SID 60110\n");
            return -1;
        }

        // Get array at list SID using delta
        CoreconfValueT* arrayValue = getCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID);
        if (arrayValue == NULL) {
            printf("Error: List array for SID 60110 not found in coreconfModel\n");
            return -1;
        }
        if (arrayValue->type != CORECONF_ARRAY) {
            printf("Error: SID 60110 is not an array in coreconfModel\n");
            return -1;
        }

        // Replace the entry whose keys all match, or append; the array takes value
        static const uint64_t keyDeltas[] = { 1 };
        if (updateCoreconfArrayByKeys(arrayValue, keyDeltas, 1, value) != 0) {
            printf("Error: Failed to update list entry for SID 60110\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60113(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 6) {
        printf("Error: SID 60113 needs 6 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];
    uint64_t targetValue_index = (uint64_t)ctx->keys->longList[ctx->keys->size - 6];

    // Call user-implemented read function
    CoreconfValueT* result = read_entry_targetValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, targetValue_index);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60113(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value is a hashmap containing keys and data
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for list item write to SID 60113\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 5) {
        printf("Error: SID 60113 needs 5 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];
    uint64_t entry_fieldId = (uint64_t)ctx->keys->longList[ctx->keys->size - 3];
    uint64_t entry_fieldPosition = (uint64_t)ctx->keys->longList[ctx->keys->size - 4];
    uint64_t entry_directionIndicator = (uint64_t)ctx->keys->longList[ctx->keys->size - 5];

    // The entry's own keys, from the written value (stored with delta SIDs)
    CoreconfValueT* targetValue_index_value = getCoreconfHashMap(value->data.map_value, 1);
    if (targetValue_index_value == NULL) {
        printf("Error: Missing key targetValue_index (SID 60114) in list item\n");
        return -1;
    }
    uint64_t targetValue_index = (uint64_t)getCoreconfValueAsUint64(targetValue_index_value);

    // Call user-implemented write function
    int result = write_entry_targetValue(rule_ruleIdValue, rule_ruleIdLength, entry_fieldId, entry_fieldPosition, entry_directionIndicator, targetValue_index, value);

    // If user function succeeded, update the datastore (list item)
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60113, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for list SID 60113\n");
            return -1;
        }

        // Get array at list SID using delta
        CoreconfValueT* arrayValue = getCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID);
        if (arrayValue == NULL) {
            printf("Error: List array for SID 60113 not found in coreconfModel\n");
            return -1;
        }
        if (arrayValue->type != CORECONF_ARRAY) {
            printf("Error: SID 60113 is not an array in coreconfModel\n");
            return -1;
        }

        // Replace the entry whose keys all match, or append; the array takes value
        static const uint64_t keyDeltas[] = { 1 };
        if (updateCoreconfArrayByKeys(arrayValue, keyDeltas, 1, value) != 0) {
            printf("Error: Failed to update list entry for SID 60113\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60116(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60116 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_fcnSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60116(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60116\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60116 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_fcnSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60116, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60116\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60116\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60117(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60117 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_fragmentationMode(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60117(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60117\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60117 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_fragmentationMode(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60117, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60117\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60117\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60118(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60118 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    CoreconfValueT* result = read_rule_inactivityTimer(rule_ruleIdValue, rule_ruleIdLength);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60118(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // A container is written as a whole: expect a map of its children
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for container write to SID 60118\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60118 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_inactivityTimer(rule_ruleIdValue, rule_ruleIdLength, value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60118, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60118\n");
            return -1;
        }

        // A write replaces: the container becomes exactly the written value
        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60118\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60119(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60119 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_inactivityTimer_ticksDuration(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60119(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60119\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60119 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_inactivityTimer_ticksDuration(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60119, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60119\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60119\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60120(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60120 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_inactivityTimer_ticksNumbers(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint16(result);
}

int handler_write_60120(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60120\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60120 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_inactivityTimer_ticksNumbers(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60120, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60120\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60120\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60121(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60121 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_l2WordSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60121(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60121\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60121 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_l2WordSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60121, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60121\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60121\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60122(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60122 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_maxAckRequests(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60122(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60122\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60122 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_maxAckRequests(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60122, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60122\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60122\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60123(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60123 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_maxInterleavedFrames(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60123(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60123\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60123 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_maxInterleavedFrames(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60123, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60123\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60123\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60124(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60124 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_maximumPacketSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint16(result);
}

int handler_write_60124(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60124\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60124 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_maximumPacketSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60124, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60124\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60124\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60125(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60125 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_rcsAlgorithm(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60125(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60125\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60125 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_rcsAlgorithm(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60125, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60125\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60125\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60126(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60126 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    CoreconfValueT* result = read_rule_retransmissionTimer(rule_ruleIdValue, rule_ruleIdLength);

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60126(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // A container is written as a whole: expect a map of its children
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for container write to SID 60126\n");
        return -1;
    }

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60126 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_retransmissionTimer(rule_ruleIdValue, rule_ruleIdLength, value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60126, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60126\n");
            return -1;
        }

        // A write replaces: the container becomes exactly the written value
        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60126\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60127(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60127 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_retransmissionTimer_ticksDuration(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60127(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60127\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60127 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_retransmissionTimer_ticksDuration(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60127, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60127\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60127\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60128(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60128 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_retransmissionTimer_ticksNumbers(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint16(result);
}

int handler_write_60128(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60128\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60128 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_retransmissionTimer_ticksNumbers(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60128, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60128\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60128\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60131(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60131 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_ruleNature(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60131(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60131\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60131 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_ruleNature(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60131, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60131\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60131\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60132(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60132 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_tileInAll1(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_60132(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60132\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60132 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_tileInAll1(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60132, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60132\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60132\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60133(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60133 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_tileSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60133(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60133\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60133 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_tileSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60133, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60133\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60133\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60134(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60134 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_wSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60134(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60134\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60134 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_wSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60134, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60134\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60134\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60135(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60135 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_rule_windowSize(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint16(result);
}

int handler_write_60135(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60135\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 60135 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_rule_windowSize(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60135, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60135\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60135\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_2000010(SIDHandlerContextT *ctx) {
    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 2000010 needs 2 keys\n");
        return NULL;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented read function
    uint64_t result = read_ietfSchcOam_proxyBehavior(rule_ruleIdValue, rule_ruleIdLength);

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint64(result);
}

int handler_write_2000010(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 2000010\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // List keys, outermost list first.  ctx->keys is read from its end: the
    // same order as ccoreconfModelExamineCoreconfValue
    if (ctx->keys == NULL || ctx->keys->size < 2) {
        printf("Error: SID 2000010 needs 2 keys\n");
        return -1;
    }
    uint64_t rule_ruleIdValue = (uint64_t)ctx->keys->longList[ctx->keys->size - 1];
    uint64_t rule_ruleIdLength = (uint64_t)ctx->keys->longList[ctx->keys->size - 2];

    // Call user-implemented write function
    int result = write_ietfSchcOam_proxyBehavior(rule_ruleIdValue, rule_ruleIdLength, nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 2000010, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 2000010\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 2000010\n");
            return -1;
        }
    }

    return result;
}

void testSchcRegisterHandlers(CoreconfModelT *model) {
    ccoreconfModelRegisterHandler(model, 60095, handler_read_60095, handler_write_60095, "/ietf-schc:schc", "void");
    ccoreconfModelRegisterHandler(model, 60096, handler_read_60096, handler_write_60096, "/ietf-schc:schc/rule", "void");
    ccoreconfModelRegisterHandler(model, 60097, handler_read_60097, handler_write_60097, "/ietf-schc:schc/rule/ack-behavior", "identityref");
    ccoreconfModelRegisterHandler(model, 60098, handler_read_60098, handler_write_60098, "/ietf-schc:schc/rule/direction", "identityref");
    ccoreconfModelRegisterHandler(model, 60099, handler_read_60099, handler_write_60099, "/ietf-schc:schc/rule/dtag-size", "uint8");
    ccoreconfModelRegisterHandler(model, 60100, handler_read_60100, handler_write_60100, "/ietf-schc:schc/rule/entry", "void");
    ccoreconfModelRegisterHandler(model, 60101, handler_read_60101, handler_write_60101, "/ietf-schc:schc/rule/entry/comp-decomp-action", "identityref");
    ccoreconfModelRegisterHandler(model, 60102, handler_read_60102, handler_write_60102, "/ietf-schc:schc/rule/entry/comp-decomp-action-value", "void");
    ccoreconfModelRegisterHandler(model, 60107, handler_read_60107, handler_write_60107, "/ietf-schc:schc/rule/entry/field-length", "['identityref', 'uint8']");
    ccoreconfModelRegisterHandler(model, 60109, handler_read_60109, handler_write_60109, "/ietf-schc:schc/rule/entry/matching-operator", "identityref");
    ccoreconfModelRegisterHandler(model, 60110, handler_read_60110, handler_write_60110, "/ietf-schc:schc/rule/entry/matching-operator-value", "void");
    ccoreconfModelRegisterHandler(model, 60113, handler_read_60113, handler_write_60113, "/ietf-schc:schc/rule/entry/target-value", "void");
    ccoreconfModelRegisterHandler(model, 60116, handler_read_60116, handler_write_60116, "/ietf-schc:schc/rule/fcn-size", "uint8");
    ccoreconfModelRegisterHandler(model, 60117, handler_read_60117, handler_write_60117, "/ietf-schc:schc/rule/fragmentation-mode", "identityref");
    ccoreconfModelRegisterHandler(model, 60118, handler_read_60118, handler_write_60118, "/ietf-schc:schc/rule/inactivity-timer", "void");
    ccoreconfModelRegisterHandler(model, 60119, handler_read_60119, handler_write_60119, "/ietf-schc:schc/rule/inactivity-timer/ticks-duration", "uint8");
    ccoreconfModelRegisterHandler(model, 60120, handler_read_60120, handler_write_60120, "/ietf-schc:schc/rule/inactivity-timer/ticks-numbers", "uint16");
    ccoreconfModelRegisterHandler(model, 60121, handler_read_60121, handler_write_60121, "/ietf-schc:schc/rule/l2-word-size", "uint8");
    ccoreconfModelRegisterHandler(model, 60122, handler_read_60122, handler_write_60122, "/ietf-schc:schc/rule/max-ack-requests", "uint8");
    ccoreconfModelRegisterHandler(model, 60123, handler_read_60123, handler_write_60123, "/ietf-schc:schc/rule/max-interleaved-frames", "uint8");
    ccoreconfModelRegisterHandler(model, 60124, handler_read_60124, handler_write_60124, "/ietf-schc:schc/rule/maximum-packet-size", "uint16");
    ccoreconfModelRegisterHandler(model, 60125, handler_read_60125, handler_write_60125, "/ietf-schc:schc/rule/rcs-algorithm", "identityref");
    ccoreconfModelRegisterHandler(model, 60126, handler_read_60126, handler_write_60126, "/ietf-schc:schc/rule/retransmission-timer", "void");
    ccoreconfModelRegisterHandler(model, 60127, handler_read_60127, handler_write_60127, "/ietf-schc:schc/rule/retransmission-timer/ticks-duration", "uint8");
    ccoreconfModelRegisterHandler(model, 60128, handler_read_60128, handler_write_60128, "/ietf-schc:schc/rule/retransmission-timer/ticks-numbers", "uint16");
    ccoreconfModelRegisterHandler(model, 60131, handler_read_60131, handler_write_60131, "/ietf-schc:schc/rule/rule-nature", "identityref");
    ccoreconfModelRegisterHandler(model, 60132, handler_read_60132, handler_write_60132, "/ietf-schc:schc/rule/tile-in-all-1", "identityref");
    ccoreconfModelRegisterHandler(model, 60133, handler_read_60133, handler_write_60133, "/ietf-schc:schc/rule/tile-size", "uint8");
    ccoreconfModelRegisterHandler(model, 60134, handler_read_60134, handler_write_60134, "/ietf-schc:schc/rule/w-size", "uint8");
    ccoreconfModelRegisterHandler(model, 60135, handler_read_60135, handler_write_60135, "/ietf-schc:schc/rule/window-size", "uint16");
    ccoreconfModelRegisterHandler(model, 2000010, handler_read_2000010, handler_write_2000010, "/ietf-schc:schc/rule/ietf-schc-oam:proxy-behavior", "identityref");
}