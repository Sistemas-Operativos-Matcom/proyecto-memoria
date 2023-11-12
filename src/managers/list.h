#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef ARRAY_LIST_H // Check if ARRAY_LIST_H is not defined
#define ARRAY_LIST_H // Define ARRAY_LIST_H

// Define a generic type for the list elements
typedef int element_type;

// Define a structure for the list
typedef struct
{
    element_type *data; // The pointer to the dynamic array
    int len;            // The current size of the list
    int size;           // The current capacity of the array
} sizeList_t;

// Declare the functions for the list operations
sizeList_t *init(); // Create a new empty list and return a pointer to it

// resetea la lista
void reset(sizeList_t *l);

// Returns the element at index i in the list
element_type get(sizeList_t *l, int i);

// Sets the value c at index i in the list
void set(sizeList_t *l, int i, element_type c);

// Increases the size of the list if necessary
void increaseSize(sizeList_t *l);

// Appends the element c at the end of the list
void push(sizeList_t *l, element_type c);

// Removes and returns the last element from the list
element_type pop(sizeList_t *l);

// Deletes the element at index i from the list
void deleteAt(sizeList_t *l, int i);

#endif // ARRAY_LIST_H
