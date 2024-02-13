#include <stdio.h>
#include <jansson.h>
#include <nanocbor/nanocbor.h>
#include "../include/serialization.h"


// Convert a generic json object to cbor
int json_to_cbor(json_t *json, nanocbor_encoder_t *cbor){
    if(json_is_object(json)){
        nanocbor_fmt_map(cbor, json_object_size(json));
        const char *key;
        json_t *value;
        json_object_foreach(json, key, value){
            nanocbor_put_tstr(cbor, key);
            json_to_cbor(value, cbor);
        }
        nanocbor_fmt_end_indefinite(cbor);
    } else if(json_is_array(json)){
        nanocbor_fmt_array(cbor, json_array_size(json));
        for(size_t i = 0; i < json_array_size(json); i++){
            json_to_cbor(json_array_get(json, i), cbor);
        }
        nanocbor_fmt_end_indefinite(cbor);
    } else if(json_is_string(json)){
        nanocbor_put_tstr(cbor, json_string_value(json));
    } else if(json_is_integer(json)){
        nanocbor_fmt_int(cbor, json_integer_value(json));
    } else if(json_is_real(json)){
        nanocbor_fmt_float(cbor, json_real_value(json));
    } else if(json_is_true(json)){
        nanocbor_fmt_bool(cbor, 1);
    } else if(json_is_false(json)){
        nanocbor_fmt_bool(cbor, 0);
    } else if(json_is_null(json)){
        nanocbor_fmt_null(cbor);
    } else {
        return -1;
    }
    return 0;
}