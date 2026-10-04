#ifndef __TEST_TEMPERATURE_SENSOR_HANDLERS_H__
#define __TEST_TEMPERATURE_SENSOR_HANDLERS_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sid_handlers.h>
#include <coreconfTypes.h>
#include <hashmap.h>
CoreconfValueT* handler_read_60001(SIDHandlerContextT *ctx);
int handler_write_60001(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60002(SIDHandlerContextT *ctx);
int handler_write_60002(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60004(SIDHandlerContextT *ctx);
int handler_write_60004(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60005(SIDHandlerContextT *ctx);
int handler_write_60005(SIDHandlerContextT *ctx, CoreconfValueT *value);

// Handler registration function
void testTemperatureSensorRegisterHandlers(CoreconfModelT *model);
#endif

/**
 * GET handler declaration
 * User implements this in the impl file to customize GET behavior
 */
CoreconfValueT* handleGetRequest(CoreconfValueT *coreconfModel);

/**
 * PUT handler declaration
 * User implements this in the impl file to customize PUT validation and behavior
 */
int handlePutRequest(CoreconfValueT **coreconfModel, CoreconfValueT *newValue, struct hashmap **clookupHashmap);
