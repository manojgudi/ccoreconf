#include <stdlib.h>
#include <libyang/libyang.h>
#include <jansson.h>
#include "hashmap.h"
#include "ccoreconf.h"
#include <cbor.h>

# define PATH_MAX_LENGTH 100
# define MAX_STACK_SIZE 100
# define SID_KEY_SIZE 21

/**
 * Stack related methods
*/

// Function to initialize the stack
void initStackStorage(StackStorageT *stackStorage, int capacity){
    stackStorage->stackElements = (StackElementT **) malloc (capacity * sizeof(StackElementT *));
    stackStorage->top = -1;
    stackStorage->capacity = capacity;
    stackStorage->size = 0;
}
// Function to check if the stack is empty
bool isEmpty(StackStorageT *stackStorage) {
    return (stackStorage->top == -1) || (stackStorage->size == 0) ;
}

// Function to check if the stack is full
bool isFull(StackStorageT* stackStorage) {
    return stackStorage->top == stackStorage->capacity - 1;
}

// Function to push an element onto the stack
void push(StackStorageT* stackStorage, StackElementT *stackElement) {
    if (isFull(stackStorage))
        fprintf(stderr, "Internal Stack overflow! Cannot push element");
    else {
        stackStorage->stackElements[++stackStorage->top] = stackElement;
        stackStorage->size++;
    }
}

// Function to pop an element from the stack
StackElementT* pop(StackStorageT *stackStorage){
    if (isEmpty(stackStorage)){
        fprintf(stderr, "Internal Stack Underflow! Cannot pop element");
        return NULL;
    } else {
        stackStorage->size--;
        return stackStorage->stackElements[stackStorage->top--];
    }
    
}

StackElementT* newStackElement(void){
    StackElementT *stackElement = (StackElementT *) malloc(sizeof(StackElementT));
    stackElement->jsonValue = json_object();
    // Initialize
    stackElement->path = malloc(sizeof(char) * PATH_MAX_LENGTH + 1);
    stackElement->parent = 0;

    return stackElement;
}

void freeStackStorage(StackStorageT *stackStorage){
    for (int i=0; i<stackStorage->size; i++){
        free(*stackStorage->stackElements+i);
    }
    free(stackStorage);
}


/**
 * Is it even required?
*/
void unwrapValues(json_t *jsonValue){
    json_t **internalStack = (json_t **) malloc(sizeof(json_t *));

    internalStack[0] = jsonValue;
    int stackSize = 1;

    while (stackSize > 0){
        json_t *stackJSONValue = internalStack[--stackSize];

        if (json_is_object(stackJSONValue)){
            const char *jsonKey;
            json_t *jsonValue;

            json_object_foreach(stackJSONValue, jsonKey, jsonValue) {
                internalStack = (json_t **)realloc(internalStack, (stackSize+1) * sizeof(json_t *));
                internalStack[stackSize++] = jsonValue;
            }
        } else if ( json_is_array(stackJSONValue)){
            size_t index;
            json_t *jsonValue;

            json_array_foreach(stackJSONValue, index, jsonValue) {
                internalStack = (json_t **)realloc(internalStack, (stackSize+1) * sizeof(json_t *));
                internalStack[stackSize++] = jsonValue;
            }

         }
    }

    free(internalStack);
}

void print_json_object(json_t *json) {
  // Check if the JSON object is null.
  if (json == NULL) {
    return;
  }

  // Print the type of the JSON object.
  switch (json_typeof(json)) {
    case JSON_NULL:
      printf("NULL\n");
      break;
    case JSON_FALSE:
      printf("FALSE\n");
      break;
    case JSON_TRUE:
      printf("TRUE\n");
      break;
    case JSON_INTEGER:
      printf("%f\n", json_number_value(json));
      break;
    case JSON_STRING:
      printf("%s\n", json_string_value(json));
      break;
    case JSON_ARRAY:
      printf("[\n");
      for (int i = 0; i < json_array_size(json); i++) {
        print_json_object(json_array_get(json, i));
      }
      printf("]\n");
      break;
    case JSON_OBJECT:
      printf("{\n");
      for (json_t *iter = json_object_iter(json); iter; iter = json_object_iter_next(json, iter)) {
        const char *key = json_object_iter_key(iter);
        json_t *value = json_object_iter_value(iter);
        printf("\t%s: ", key);
        print_json_object(value);
      }
      printf("}\n");
      break;
  }
}

// Assume long value is 64bit long, 
void long2str(char* stringValue, long longValue){
    sprintf(stringValue, "%ld", longValue);
}

/**
 * Lookup SID
*/

// Assume *path = "/" and parent = 0 when this function is being invoked
void lookupSID(json_t *jsonValue, SIDModelT *sidModel){

    char* path = "/";
    long parent = 0;

    StackStorageT *stackStorage = (StackStorageT *) malloc(sizeof(StackStorageT));
    initStackStorage(stackStorage, MAX_STACK_SIZE);

    StackElementT *initialStackElement = newStackElement();
    StackElementT *currentStackElement;
    initialStackElement->jsonValue = jsonValue;
    initialStackElement->path = path;
    initialStackElement->parent = parent;

    char* currentPath = NULL;
    int currentParent = 0;
    json_t* currentObject = NULL;

    push(stackStorage, initialStackElement);
    while (!isEmpty(stackStorage)){

        printf("Original JSON Obj \n");
        print_json_object(jsonValue);
        printf("---------\n");

        currentStackElement = pop(stackStorage);
        currentPath = malloc( sizeof(char) * PATH_MAX_LENGTH);
        strcpy(currentPath, currentStackElement->path);
        currentParent = currentStackElement->parent;

        //currentObject = (currentStackElement->jsonValue);

        printf("Before\n");
        print_json_object(currentStackElement->jsonValue);
        if ( json_is_object(currentStackElement->jsonValue)){
            const char *key;
            json_t *value;

            // TODO Improve the logic, you ideally should never need to do this. Get a list of keys
            // We are doing this since we are mutating json_t object
            json_t *jsonKeys = json_array();
            json_t *iter = json_object_iter(currentStackElement->jsonValue);
            while(iter){
                const char* key_ = json_object_iter_key(iter);
                // Add key to the list of keys
                json_array_append(jsonKeys, json_string(key_));

                iter = json_object_iter_next(currentStackElement->jsonValue, iter);
            }

            //json_object_foreach(currentStackElement->jsonValue, key, value) {
            for (int i =0; i<json_array_size(jsonKeys); i++){

                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = currentStackElement->jsonValue;
                inStackElement->path = currentStackElement->path;
                inStackElement->parent = inStackElement->parent;


                key = json_string_value(json_array_get(jsonKeys, i));
                value = json_object_get(currentStackElement->jsonValue, key);

                char *qualifiedPath = malloc( sizeof(char) * PATH_MAX_LENGTH);
                strcpy(qualifiedPath, currentPath);
                strcat(qualifiedPath, key);
                //    user = hashmap_get(map, &(struct user){ .name="Tom" });
                IdentifierSIDT *identifierSID = hashmap_get(sidModel->identifierSIDHashMap, &(IdentifierSIDT){.identifier= qualifiedPath});
                if (!identifierSID){
                    fprintf(stderr, "Following qualified path should be in the map but wasn't found %s\n", qualifiedPath);
                    return;
                }
                
                long childSIDValue = identifierSID->sid;
                long sidDiff = childSIDValue - currentParent;
                char sidKey[SID_KEY_SIZE] = "";
                long2str(sidKey, sidDiff);

                // Set the new parent
                inStackElement->parent = childSIDValue;

                // Set the sidDiff value line 160
                // If value is not 0 (0 is return for success), then
                if (json_object_set(inStackElement->jsonValue, 
                    sidKey, value) < 0)
                        fprintf(stderr, "Couldn't update the json value");
                
                // Delete the existing key, equivalent to pop() line 160 in pycoreconf.
                if (json_object_del(inStackElement->jsonValue, key) < 0)
                    fprintf(stderr, "Key %s was not found in the json, this should not happen\n", key);

                // NOTE This function shows that we're putting an extra K:V, put it on line 199 and check if its working properly
                printf("After\n");
                print_json_object(inStackElement->jsonValue);
                //jsonValue = currentStackElement->jsonValue;

                // NOTE fix this line, should it get from currentObject or currentStackElement->jsonValue?
                inStackElement->jsonValue = json_object_get(currentStackElement->jsonValue, sidKey);
                
                printf("Path %s\n", inStackElement->path);
                // Dump the content and allot new
                strcat(qualifiedPath, "/");
                inStackElement->path = malloc(sizeof(char) * PATH_MAX_LENGTH);
                strcpy(inStackElement->path, qualifiedPath);
                push(stackStorage, inStackElement);
                //currentStackElement->jsonValue
                free(qualifiedPath);

            }
            
            json_decref(jsonKeys);
            
        } else if (json_is_array(currentStackElement->jsonValue)){
            size_t arrayLength = json_array_size(currentStackElement->jsonValue);
            for (int i=0; i < arrayLength; i++){
                StackElementT *inStackElement = newStackElement();
                inStackElement->jsonValue = json_array_get(currentStackElement->jsonValue, i);
                inStackElement->parent = currentStackElement->parent;
                inStackElement->path = currentStackElement->path;
                push(stackStorage, inStackElement);
            }
        }

        else {
            char* formattedPath = malloc(sizeof(char) * PATH_MAX_LENGTH);
            formatPath(currentStackElement->path, formattedPath);

            printHashMap(sidModel->identifierTypeHashMap, IDENTIFIER_TYPE);
            IdentifierTypeT *identifierType = hashmap_get(sidModel->identifierTypeHashMap, &(IdentifierTypeT){.identifier = formattedPath});
            if (identifierType == NULL){
                fprintf(stderr, "No valid identifier found for the formatted qualified path %s\n", formattedPath);
                return;
            }
            //enum SchemaIdentifierTypeEnum sidType = identifierType->type;
            free(formattedPath);
        }
        // check for list, test the dict part first
     
    }
   
    printf("Finalment: \n");
    print_json_object(jsonValue);
    printf("---------\n");
    free(currentPath);

    
}


void convertToCBORType(json_t *jsonItem, enum SchemaIdentifierTypeEnum identifierType, cbor_item_t *cborItem){
    size_t jsonStringLength;
    switch (identifierType){
        // For null terminated UTF-8 string
        case STRING:
            // Deallocate previous cborItem as cbor_build_stringn allocates new memory
            free(cborItem);
            cborItem = NULL;
            const char* jsonValue = json_string_value(jsonItem);
            jsonStringLength = json_string_length(jsonItem);
            cborItem = cbor_build_stringn(jsonValue, jsonStringLength);
            break;
        case UINT_8:
            uint8_t jsonValue_8i = (uint8_t) json_integer_value(jsonItem);
            cbor_set_uint8(cborItem, jsonValue_8i);
            break;
        case UINT_16:
            uint16_t jsonValue_16i = (uint16_t) json_integer_value(jsonItem);
            cbor_set_uint16(cborItem, jsonValue_16i);
            break;
        case UINT_32:
            uint32_t jsonValue_32i = (uint32_t) json_integer_value(jsonItem);
            cbor_set_uint32(cborItem, jsonValue_32i);
            break;
        case UINT_64:
            uint64_t jsonValue_64i = (uint64_t) json_integer_value(jsonItem);
            cbor_set_uint64(cborItem, jsonValue_64i);
            break;
        case BOOLEAN:
            bool jsonValue_b = json_boolean_value(jsonItem);
            cbor_set_bool(cborItem, jsonValue_b);
        default:
            // Treat it as a string
            // Deallocate previous cborItem as cbor_build_stringn allocates new memory
            free(cborItem);
            cborItem = NULL;
            const char* jsonValue_d = json_string_value(jsonItem);
            jsonStringLength = json_string_length(jsonItem);
            cborItem = cbor_build_stringn(jsonValue_d, jsonStringLength);
            break;

    }

}


// use an indefinite map to create a new cbormap
//cbor_item_t *cborIMap = cbor_new_indefinite_map();
void convertToCORECONF(cbor_item_t *cborIMap, json_t *jsonMap){
    if (!json_is_object(jsonMap))
        return;
    
    // Iterate over the map
    
}