#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef ARRAY_LIST_H // Check if ARRAY_LIST_H is not defined
#define ARRAY_LIST_H // Define ARRAY_LIST_H

// Define a generic type for the list elements
typedef int element_type;

// Define a structure for the list
typedef struct {
    element_type* array; // The pointer to the dynamic array
    int size; // The current size of the list
    int capacity; // The current capacity of the array
} array_list;

// Declare the functions for the list operations
array_list* create_list(); // Create a new empty list and return a pointer to it
bool insert(array_list*, element_type, int); // Insert an element at a given position in the list
bool append(array_list*, element_type); // Append an element at the end of the list
bool delete(array_list*, int); // Delete an element at a given position in the list
void print_list(array_list*); // Print the elements of the list
void free_list(array_list*); // Free the memory allocated for the list

#endif // ARRAY_LIST_H
