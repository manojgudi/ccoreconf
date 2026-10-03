#include "../include/sid.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/hashmap.h"

DynamicLongListT *createDynamicLongList(void) {
    DynamicLongListT *dynamicLongList = (DynamicLongListT *)malloc(sizeof(DynamicLongListT));

    if (dynamicLongList == NULL) {
        fprintf(stderr, "Failed malloc'ing Dynamic long list");
        return NULL;
    }
    dynamicLongList->longList = NULL;
    dynamicLongList->size = 0;
    return dynamicLongList;
}

// Empty a list made by createDynamicLongList (or already initialized), freeing
// its old buffer.  addLong allocates on demand: realloc(NULL, n) is malloc(n).
void initializeDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, do nothing;
    if (!dynamicLongList) return;
    free(dynamicLongList->longList);
    dynamicLongList->longList = NULL;
    dynamicLongList->size = 0;
}

// Comparison function for qsort
int compareLong(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

// Function to sort the dynamic long list, it assumes sortedArray is already correctly inititliazed to
// dynamicLongList->size
void sortDynamicLongList(DynamicLongListT *dynamicLongList, long sortedArray[]) {
    // populate the sortedArray with dynamicLongList values
    for (size_t i = 0; i < dynamicLongList->size; i++) {
        sortedArray[i] = dynamicLongList->longList[i];
    }

    // Sort the array
    qsort(sortedArray, dynamicLongList->size, sizeof(long), compareLong);
    // Add the sorted array to the sortedDynamicLongList
}

// Function to compare two dynamicLongList which converts to a sorted long array and then compares the array
bool compareDynamicLongList(DynamicLongListT *dynamicLongList1, DynamicLongListT *dynamicLongList2) {
    size_t array1Size = dynamicLongList1->size;
    size_t array2Size = dynamicLongList2->size;

    // Check if either of the dynamicLongLists have size = 0
    if ((array1Size == 0) || (array2Size == 0)) return false;

    if (array1Size != array2Size) return false;

    // Created two long arrays for sorting with size of dynamicLongLists
    long array1[array1Size];
    long array2[array1Size];

    // sort the two dynamicLongLists
    sortDynamicLongList(dynamicLongList1, array1);
    sortDynamicLongList(dynamicLongList2, array2);

    // Compare the two arrays, and return true if array1 is exactly the same as array2
    for (size_t i = 0; i < array1Size; i++) {
        if (array1[i] != array2[i]) return false;
    }

    return true;
}

void addLong(DynamicLongListT *dynamicLongList, long value) {
    size_t currentListSize;
    if (dynamicLongList == NULL) {
        currentListSize = 0;
    } else {
        currentListSize = dynamicLongList->size;
    }

    dynamicLongList->longList = (long *)realloc(dynamicLongList->longList, (currentListSize + 1) * sizeof(long));
    // Check if realloc happened properly, if no, then realloc failed and longList will be NULL
    if (!dynamicLongList->longList) {
        fprintf(stderr, "Failed realloc'ing long list");
        return;
    }
    dynamicLongList->size = currentListSize + 1;
    dynamicLongList->longList[currentListSize] = value;
}

// get the last value from dynamicLongList
// with an option to remove the item from the list
long getLong(DynamicLongListT *dynamicLongList, bool removeFromList) {
    // If its NULL, then do nothing
    if (!dynamicLongList) return 0;
    // If the list is empty, then return -1
    if (dynamicLongList->size == 0) return 0;
    long lastValue = dynamicLongList->longList[dynamicLongList->size - 1];
    if (removeFromList == true) {
        dynamicLongList->longList =
            (long *)realloc(dynamicLongList->longList, (dynamicLongList->size - 1) * sizeof(long));
        dynamicLongList->size = dynamicLongList->size - 1;
    }
    return lastValue;
}

// pop the last value from dynamicLongList
long popLong(DynamicLongListT *dynamicLongList) { return getLong(dynamicLongList, true); }

// peek the last value from dynamicLongList, do not remove it
long peekLong(DynamicLongListT *dynamicLongList) { return getLong(dynamicLongList, false); }

// Clone a DynamicLongListT
void cloneDynamicLongList(DynamicLongListT *originalDynamicLongList, DynamicLongListT *clonedDynamicLongList) {
    // If its NULL, then do nothing
    if (originalDynamicLongList == NULL || originalDynamicLongList->size == 0) return;

    // Initialize the clonedDynamicLongList
    initializeDynamicLongList(clonedDynamicLongList);
    // Iterate over the originalDynamicLongList and add all the values to the clonedDynamicLongList
    for (size_t i = 0; i < originalDynamicLongList->size; i++) {
        addLong(clonedDynamicLongList, originalDynamicLongList->longList[i]);
    }
}

void addUniqueLong(DynamicLongListT *dynamicLongList, long value) {
    // Check if the value already exists in the list
    for (size_t i = 0; i < dynamicLongList->size; i++) {
        if (dynamicLongList->longList[i] == value) {
            return;
        }
    }
    addLong(dynamicLongList, value);
}

void printDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, then print []
    if (!dynamicLongList) {
        printf("[]");
        return;
    }
    printf("DLL: ");
    for (size_t i = 0; i < dynamicLongList->size; i++) {
        printf("%lu, ", dynamicLongList->longList[i]);
    }
    printf("\n");
}

void freeDynamicLongList(DynamicLongListT *dynamicLongList) {
    // If its NULL, then do nothing
    if (!dynamicLongList) return;
    free(dynamicLongList->longList);
    free(dynamicLongList);
}

int keyMappingCompare(const void *a, const void *b, void *udata) {
    // NOTE Keep it unused for compatibility reasons
    (void)udata;

    const KeyMappingT *keyMapping1 = (KeyMappingT *)a;
    const KeyMappingT *keyMapping2 = (KeyMappingT *)b;
    // return strcmp(keyMapping1->key, keyMapping2->key);
    return (keyMapping1->key != keyMapping2->key);
}

uint64_t keyMappingHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const KeyMappingT *keyMapping = (KeyMappingT *)item;
    return hashmap_murmur(&(keyMapping->key), sizeof(uint64_t), seed0, seed1);
}

void keyMappingFree(void *item) {
    KeyMappingT *keyMapping = (KeyMappingT *)item;
    if (keyMapping && keyMapping->dynamicLongList) {
        freeDynamicLongList(keyMapping->dynamicLongList);
        keyMapping->dynamicLongList = NULL;
    }
}

void printKeyMappingT(const KeyMappingT *keyMapping) {
    printf("\nFor the key %d: \n", (int)keyMapping->key);

    // Iterate over DynamicLongListT
    for (size_t i = 0; i < keyMapping->dynamicLongList->size; i++) {
        long childSID = *(keyMapping->dynamicLongList->longList + i);
        printf("%lu, ", childSID);
    }
}

void printKeyMappingHashMap(struct hashmap *keyMappingHashMap) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(keyMappingHashMap, &iter, &item)) {
        const KeyMappingT *keyMapping = item;
        printKeyMappingT(keyMapping);
    }
}

/*
Convert char* to int64_t, return INTMAX_MIN in case of an error
*/
int64_t char2int64(char *keyString) {
    // Convert char* to int64_t using strtoimax
    intmax_t intValue = strtoimax(keyString, NULL, 10);
    if (intValue == INTMAX_MIN || intValue == INTMAX_MAX) {
        fprintf(stderr, "Conversion error or out of range");
        return INTMAX_MIN;
    }

    // Check for valid conversion
    if (errno == ERANGE) {
        fprintf(stderr, "Value out of range");
        return INTMAX_MIN;
    }

    // Convert intmax_t to int64_t
    int64_t int64Value = (int64_t)intValue;

    return int64Value;
}

/*
  Convert char* to uint64_t, return UINTMAX_MIN in case of an error
*/
uint64_t char2uint64(char *keyString) {
    // Convert char* to uint64_t using strtoumax
    uintmax_t uintValue = strtoumax(keyString, NULL, 10);
    if (uintValue == UINTMAX_MAX) {
        fprintf(stderr, "Conversion error or out of range");
        return 0;
    }

    // Check for valid conversion
    if (errno == ERANGE) {
        fprintf(stderr, "Value out of range");
        return 0;
    }

    // Convert uintmax_t to uint64_t
    uint64_t uint64Value = (uint64_t)uintValue;

    return uint64Value;
}
