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

// Initializes the process_List_t structure and returns a pointer to it
process_List_t *p_init();

// resetea la lista
void p_reset(process_List_t *l);

// Returns the element at index i in the list
process_pag_t *p_get(process_List_t *l, int i);

// Sets the value c at index i in the list
int p_set(process_List_t *l, int i, process_pag_t *c);

// Checks if the index i is within the valid range of the list
int p_validIndex(process_List_t *l, int i);

// Increases the size of the list if necessary
void p_increaseSize(process_List_t *l);

// Inserts the element c at index i in the list
int p_insert(process_List_t *l, int i, process_pag_t *c);

// Appends the element c at the end of the list
void p_push(process_List_t *l, process_pag_t *c);

// Removes and returns the last element from the list
process_pag_t *p_pop(process_List_t *l);

// Deletes the element at index i from the list
int p_deleteAt(process_List_t *l, int i);

#endif