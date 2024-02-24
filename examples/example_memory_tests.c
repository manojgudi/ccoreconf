#include <stdio.h>
#include <stdlib.h>

/**
 *  Key takeaways:
 * if pointer is already malloced then write a function with void return
 * if pointer is not allocted then write a function which mallocs and returns the pointer
*/

// Function to manipulate a pointer
void manipulatePointer(int *ptr) {
    // Dereference the pointer and manipulate its value
    *ptr = 100;
}

// DONT DO THIS
void manipulatePointerMem(int *ptr){
    // Allocate memory to pointer 
    ptr = (int *)malloc(sizeof(int));
    // Dereference the pointer and manipulate its value
    *ptr = 101;
}

int* manipulatePointerWithReturn(){
    // Create a new int pointer in heap assign 102 value to it and return
    int *ptr = (int *)malloc(sizeof(int));
    *ptr = 102;
    return ptr;
}

int main() {
    int *ptr, *ptr2;

    // Allocate memory
    ptr = (int *)malloc(sizeof(int));

    // Check for successful allocation
    if (ptr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Call the function to manipulate the pointer
    manipulatePointer(ptr);
    //manipulatePointerMem(ptr2); // Invalid as ptr2 is not allocated in heap

    int *ptr3 = manipulatePointerWithReturn();

    // Print the manipulated value
    printf("Value at ptr: %d\n", *ptr);
    //printf("Value at ptr2: %d\n", *ptr2);
    printf("Value at ptr3: %d\n", *ptr3);

    // Free allocated memory
    free(ptr);

    // CANNOT free ptr2 as it is not allocated in heap
    //free(ptr2);
    free(ptr3);

    return 0;
}
