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

#include <stddef.h>

typedef struct process_List
{
    size_t len;           // current size of the array
    process_pag_t **data; // posize_ter to the array of process
    size_t size;          // maximum size that can be stored
} process_List_t;

// Initializes the process_List_t structure and returns a posize_ter to it
process_List_t *p_init();

// resetea la lista
void p_reset(process_List_t *l);

// Returns the element at index i in the list
process_pag_t *p_get(process_List_t *l, size_t i);

// Sets the value c at index i in the list
void p_set(process_List_t *l, size_t i, process_pag_t *c);

// Increases the size of the list if necessary
void p_increaseSize(process_List_t *l);

// Appends the element c at the end of the list
void p_push(process_List_t *l, process_pag_t *c);

// Removes and returns the last element from the list
process_pag_t *p_pop(process_List_t *l);

// Deletes the element at index i from the list
void p_deleteAt(process_List_t *l, size_t i);

#endif