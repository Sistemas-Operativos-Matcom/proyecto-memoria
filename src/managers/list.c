#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"

// Create a new empty list and return a pointer to it
array_list* create_list() {
    array_list* list = malloc(sizeof(array_list));
    if (list == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    list->size = 0; // Initialize the size to zero
    list->capacity = 10; // Initialize the capacity to some initial value
    // Allocate memory for the array using calloc
    list->array = calloc(list->capacity, sizeof(element_type));
    if (list->array == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    return list;
}

// Insert an element at a given position in the list
// Return true if successful, false if the position is invalid
bool insert(array_list* list, element_type element, int position) {
    if (position < 0 || position > list->size) {
        printf("Invalid position\n");
        return false;
    }
    // Check if the array is full
    if (list->size == list->capacity) {
        // Increase the capacity by a factor of 2
        int new_capacity = list->capacity * 2;
        // Reallocate memory for the array using realloc
        element_type* new_array = realloc(list->array, new_capacity * sizeof(element_type));
        if (new_array == NULL) {
            printf("Memory allocation failed\n");
            return false;
        }
        // Update the pointer, capacity and size of the list
        list->array = new_array;
        list->capacity = new_capacity;
    }
    // Shift the elements from the position to the end by one position to the right
    for (int i = list->size - 1; i >= position; i--) {
        list->array[i + 1] = list->array[i];
    }
    // Insert the element at the position
    list->array[position] = element;
    // Increment the size of the list
    list->size++;
    return true;
}

// Append an element at the end of the list
// Return true if successful, false if memory allocation fails
bool append(array_list* list, element_type element) {
    return insert(list, element, list->size); // Insert at the last position
}

// Delete an element at a given position in the list
// Return true if successful, false if the position is invalid or the list is empty
bool delete(array_list* list, int position) {
    if (position < 0 || position >= list->size) {
        printf("Invalid position\n");
        return false;
    }
    if (list->size == 0) {
        printf("List is empty\n");
        return false;
    }
    // Shift the elements from the position + 1 to the end by one position to the left
    for (int i = position + 1; i < list->size; i++) {
        list->array[i - 1] = list->array[i];
    }
    // Decrement the size of the list
    list->size--;
    // Check if the array is too sparse
    if (list->size < list->capacity / 4) {
        // Decrease the capacity by a factor of 2
        int new_capacity = list->capacity / 2;
        // Reallocate memory for the array using realloc
        element_type* new_array = realloc(list->array, new_capacity * sizeof(element_type));
        if (new_array == NULL) {
            printf("Memory allocation failed\n");
            return false;
        }
        // Update the pointer, capacity and size of the list
        list->array = new_array;
        list->capacity = new_capacity;
    }
    return true;
}

// Print the elements of the list
void print_list(array_list* list) {
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%d", list->array[i]);
        if (i < list->size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Free the memory allocated for the list
void free_list(array_list* list) {
    free(list->array); // Free the memory allocated for the array
    free(list); // Free the memory allocated for the list structure
}
