#include "fileOperations.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

json_t *readJSON(const char *filePath) {
    json_t *jsonFromFile = NULL;

    long fileSize;
    // SID JSON file operations
    FILE *fp = fopen(filePath, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open the file: %s\n", filePath);
        fclose(fp);
        return jsonFromFile;
    }

    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer
    char *fileBuffer = (char *)malloc(fileSize + 1);
    if (!fileBuffer) {
        fprintf(stderr, "Failed allotting memory for the buffer.\n");
        return jsonFromFile;
    }

    // Read the entire file into the buffer
    size_t bytesRead = fread(fileBuffer, 1, fileSize, fp);
    if (!bytesRead) {
        fprintf(stderr, "Failed to read the file: %s\n", filePath);
        fclose(fp);
        free(fileBuffer);
        return jsonFromFile;
    } else {
        printf("File content read %zu", bytesRead);
    }

    // Parse the JSON File
    fileBuffer[fileSize] = '\0';
    json_error_t error;
    jsonFromFile = json_loads(fileBuffer, 0, &error);
    if (!jsonFromFile) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
    }

    fclose(fp);
    free(fileBuffer);
    return jsonFromFile;
}