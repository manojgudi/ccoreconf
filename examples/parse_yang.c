#include <stdio.h>
#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>

/*Example to read SID file and find a SID and its corresponding value*/
void main(){

    long fileSize;
    const char* jsonFilePath = "/home/valentina/projects/lpwan_examples/ccoreconf/samples/sid_examples/ietf-schc@2022-12-19.sid";
    const char* keyMappingString = "key-mapping";

    FILE *fp = fopen(jsonFilePath, "r");
    if (!fp){
        fprintf(stderr, "Failed to open the file: %s\n", jsonFilePath);
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer
    char* sidFileBuffer = (char*) malloc(fileSize + 1);
    if (!sidFileBuffer){
        fprintf(stderr, "Failed allotting memory for the buffer.\n");
        fclose(fp);
        return;
    }

    // Read the entire file into the buffer
    size_t bytesRead = fread(sidFileBuffer, 1, fileSize, fp);
    if (!bytesRead){
        fprintf(stderr, "Failed to read the file: %s\n", jsonFilePath);
        fclose(fp);
        return;
    } else {
        printf("File content read %zu", bytesRead);
    }

    // Parse the JSON File
    sidFileBuffer[fileSize] = '\0';
    json_error_t error;
    json_t* root = json_loads(sidFileBuffer, 0, &error);
    if (!root){
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        fclose(fp);
        return;
    }

    // Access key-mapping
    json_t* keyMappingMap = json_object_get(root, keyMappingString);
    if (!json_is_object(keyMappingMap)){
        fprintf(stderr, "Failed %s does not return a JSON map:", keyMappingString);
        fclose(fp);
        return;
    }

    // Iterate over the json
    const char* key;
    json_t* value;
    json_object_foreach(keyMappingMap, key, value){
        if (!json_is_array(value)){
            printf("Key %s and the value %s \n", key, json_string_value(value));
        } else {
            printf("Found an array, printing it: \n");
            for (int i=0; i < json_array_size(value); i++){
                json_t *data = json_array_get(value, i);
                if (json_is_number(data)){
                    printf("For key %s print %lu\n",key, (long) json_number_value(data));  
                }

                }
        }
    }

    // Cleanup
    json_decref(root);
    free(sidFileBuffer);
}



