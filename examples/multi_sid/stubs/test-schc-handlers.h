#ifndef __TEST_SCHC_HANDLERS_H__
#define __TEST_SCHC_HANDLERS_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sid_handlers.h>
#include <coreconfTypes.h>
#include <hashmap.h>
CoreconfValueT* handler_read_60095(SIDHandlerContextT *ctx);
int handler_write_60095(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60096(SIDHandlerContextT *ctx);
int handler_write_60096(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60097(SIDHandlerContextT *ctx);
int handler_write_60097(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60098(SIDHandlerContextT *ctx);
int handler_write_60098(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60099(SIDHandlerContextT *ctx);
int handler_write_60099(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60100(SIDHandlerContextT *ctx);
int handler_write_60100(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60101(SIDHandlerContextT *ctx);
int handler_write_60101(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60102(SIDHandlerContextT *ctx);
int handler_write_60102(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60107(SIDHandlerContextT *ctx);
int handler_write_60107(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60109(SIDHandlerContextT *ctx);
int handler_write_60109(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60110(SIDHandlerContextT *ctx);
int handler_write_60110(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60113(SIDHandlerContextT *ctx);
int handler_write_60113(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60116(SIDHandlerContextT *ctx);
int handler_write_60116(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60117(SIDHandlerContextT *ctx);
int handler_write_60117(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60118(SIDHandlerContextT *ctx);
int handler_write_60118(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60119(SIDHandlerContextT *ctx);
int handler_write_60119(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60120(SIDHandlerContextT *ctx);
int handler_write_60120(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60121(SIDHandlerContextT *ctx);
int handler_write_60121(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60122(SIDHandlerContextT *ctx);
int handler_write_60122(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60123(SIDHandlerContextT *ctx);
int handler_write_60123(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60124(SIDHandlerContextT *ctx);
int handler_write_60124(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60125(SIDHandlerContextT *ctx);
int handler_write_60125(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60126(SIDHandlerContextT *ctx);
int handler_write_60126(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60127(SIDHandlerContextT *ctx);
int handler_write_60127(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60128(SIDHandlerContextT *ctx);
int handler_write_60128(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60131(SIDHandlerContextT *ctx);
int handler_write_60131(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60132(SIDHandlerContextT *ctx);
int handler_write_60132(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60133(SIDHandlerContextT *ctx);
int handler_write_60133(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60134(SIDHandlerContextT *ctx);
int handler_write_60134(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_60135(SIDHandlerContextT *ctx);
int handler_write_60135(SIDHandlerContextT *ctx, CoreconfValueT *value);
CoreconfValueT* handler_read_2000010(SIDHandlerContextT *ctx);
int handler_write_2000010(SIDHandlerContextT *ctx, CoreconfValueT *value);

// Handler registration function
void testSchcRegisterHandlers(CoreconfModelT *model);
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
