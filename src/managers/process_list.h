#ifndef PROCESS_LIST_H // Directiva de preprocesador para evitar la inclusión múltiple
#define PROCESS_LIST_H

#include "pag_manager.h"
#include "stdio.h"
#include "stack.h"
#include "process_pag.h"
#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include "../tests.h"
#include "../memory.c"

typedef struct process_List
{
    int len;              // current size of the array
    process_pag_t **data; // pointer to the array of process
    size_t size;          // maximum size that can be stored
} process_List_t;

// Returns the length (number of elements) of the list
int length(process_List_t *list);

// Initializes the process_List_t structure and returns a pointer to it
process_List_t *init();

// resetea la lista
void reset(process_List_t *l);

// Returns the element at index i in the list
process_pag_t *get(process_List_t *l, int i);

// Sets the value c at index i in the list
int set(process_List_t *l, int i, process_pag_t *c);

// Checks if the index i is within the valid range of the list
int validIndex(process_List_t *l, int i);

// Increases the size of the list if necessary
void increaseSize(process_List_t *l);

// Inserts the element c at index i in the list
int insert(process_List_t *l, int i, process_pag_t *c);

// Appends the element c at the end of the list
void push(process_List_t *l, size_t c);

// Removes and returns the last element from the list
process_pag_t *pop(process_List_t *l);

// Prints all the elements in the list
void printAll(process_List_t *l);

// Prints the element at index i in the list
void printAt(process_List_t *l, int i);

// Deletes the element at index i from the list
int deleteAt(process_List_t *l, int i);

#endif