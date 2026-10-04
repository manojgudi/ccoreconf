#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "test-temperature-sensor-handlers.h"
#include "test-temperature-sensor-impl.h"
#include "sid_handlers.h"
#include "coreconfTypes.h"
#include "coreconfManipulation.h"
#include "coreconfModel.h"
#include "dynamicLongList.h"
CoreconfValueT* handler_read_60001(SIDHandlerContextT *ctx) {
    // Call user-implemented read function
    CoreconfValueT* result = read_TemperatureSensor_TemperatureSensorObject();

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60001(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // A container is written as a whole: expect a map of its children
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for container write to SID 60001\n");
        return -1;
    }

    // Call user-implemented write function
    int result = write_TemperatureSensor_TemperatureSensorObject(value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60001, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60001\n");
            return -1;
        }

        // A write replaces: the container becomes exactly the written value
        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60001\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60002(SIDHandlerContextT *ctx) {
    // Call user-implemented read function
    uint64_t result = read_TemperatureSensorObject_batteryLevel();

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfUint8(result);
}

int handler_write_60002(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (!isTypeUint(value->type)) {
        printf("Error: Expected unsigned integer type for SID 60002\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    uint64_t nativeValue = getCoreconfValueAsUint64(value);

    // Call user-implemented write function
    int result = write_TemperatureSensorObject_batteryLevel(nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60002, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60002\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60002\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60004(SIDHandlerContextT *ctx) {
    // Call user-implemented read function
    CoreconfValueT* result = read_TemperatureSensorObject_temperature();

    if (result == NULL) {
        return ccoreconfModelExamineCoreconfValue(ctx->model, ctx->keys, ctx->pathNode);
    } else {
        return result;
    }
}

int handler_write_60004(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // A container is written as a whole: expect a map of its children
    if (value->type != CORECONF_HASHMAP) {
        printf("Error: Expected hashmap for container write to SID 60004\n");
        return -1;
    }

    // Call user-implemented write function
    int result = write_TemperatureSensorObject_temperature(value);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60004, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60004\n");
            return -1;
        }

        // A write replaces: the container becomes exactly the written value
        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60004\n");
            return -1;
        }
    }

    return result;
}

CoreconfValueT* handler_read_60005(SIDHandlerContextT *ctx) {
    // Call user-implemented read function
    double result = read_temperature_voltageADC();

    // Return the user's value; the caller owns (and frees) the new CoreconfValueT
    return createCoreconfReal(result);
}

int handler_write_60005(SIDHandlerContextT *ctx, CoreconfValueT *value) {
    // Validate value type
    if (value->type != CORECONF_REAL) {
        printf("Error: Expected type CORECONF_REAL for SID 60005\n");
        return -1;
    }

    // Extract native value from CoreconfValueT
    double nativeValue = value->data.real_value;

    // Call user-implemented write function
    int result = write_temperature_voltageADC(nativeValue);

    // If user function succeeded, update the datastore
    if (result == 0) {
        uint64_t finalDeltaSID = 0;
        CoreconfValueT* parentContainer = navigateToParentContainer(
            ctx->model->root, ctx->keys, ctx->pathNode, 60005, &finalDeltaSID);

        if (parentContainer == NULL) {
            printf("Error: Failed to navigate to parent for SID 60005\n");
            return -1;
        }

        if (insertCoreconfHashMap(parentContainer->data.map_value, finalDeltaSID, value) != 0) {
            printf("Error: Failed to update coreconfModel for SID 60005\n");
            return -1;
        }
    }

    return result;
}

void testTemperatureSensorRegisterHandlers(CoreconfModelT *model) {
    ccoreconfModelRegisterHandler(model, 60001, handler_read_60001, handler_write_60001, "/TemperatureSensor:TemperatureSensorObject", "void");
    ccoreconfModelRegisterHandler(model, 60002, handler_read_60002, handler_write_60002, "/TemperatureSensor:TemperatureSensorObject/batteryLevel", "uint8");
    ccoreconfModelRegisterHandler(model, 60004, handler_read_60004, handler_write_60004, "/TemperatureSensor:TemperatureSensorObject/temperature", "void");
    ccoreconfModelRegisterHandler(model, 60005, handler_read_60005, handler_write_60005, "/TemperatureSensor:TemperatureSensorObject/temperature/voltageADC", "decimal64");
}