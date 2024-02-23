#include <stdio.h>
#include <jansson.h>
#include <nanocbor/nanocbor.h>

int main() {
    // Create a JSON object using Jansson
    json_t *jsonObj = json_object();
    json_object_set_new(jsonObj, "name", json_string("John Doe"));
    json_object_set_new(jsonObj, "age", json_integer(30));
    json_object_set_new(jsonObj, "is_student", json_true());

    // Encode the JSON object into CBOR format using NanoCBOR
    size_t cbor_buffer_size = 1024; // Adjust as needed
    uint8_t cbor_buffer[cbor_buffer_size];
    nanocbor_encoder_t encoder;
    nanocbor_encoder_init(&encoder, cbor_buffer, cbor_buffer_size);
    
    // Start encoding the JSON object
    nanocbor_fmt_map(&encoder, 3);
    nanocbor_put_tstr(&encoder, "name");
    nanocbor_put_tstr(&encoder, json_string_value(json_object_get(jsonObj, "name")));
    nanocbor_put_tstr(&encoder, "age");
    nanocbor_fmt_int(&encoder, (int64_t)json_integer_value(json_object_get(jsonObj, "age")));
    nanocbor_put_tstr(&encoder, "is_student");
    nanocbor_fmt_bool(&encoder, json_is_true(json_object_get(jsonObj, "is_student")));
    nanocbor_fmt_end_indefinite(&encoder);
    //nanocbor_fmt_end(&encoder);

    // Print the encoded CBOR data
    printf("Encoded CBOR data:\n");
    for (size_t i = 0; i < nanocbor_encoded_len(&encoder); i++) {
        printf("%02x ", cbor_buffer[i]);
    }
    printf("\n");

    // Cleanup
    json_decref(jsonObj);

    return 0;
}
