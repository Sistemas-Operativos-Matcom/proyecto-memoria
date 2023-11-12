#ifndef PROCESS_H
#define PROCESS_H

#include "../memory.h"
#include "../utils.h"
#include "list.h"
#include "heap.h"
#include "stack.h"
#include <stddef.h>

typedef struct virtual_mem
{
    heap_t *heap;
    stack_t *stack;
} virtual_mem_t;

typedef struct process_pag
{
    process_t process;
    sizeList_t *pages_table;
    virtual_mem_t *v_memory;
} process_pag_t;

// Constructor for process_pag_t
 process_pag_t *init_process_pag(process_t process);

// Constructor for virtual_mem_t
 virtual_mem_t *init_virtual_mem(process_t process);

#endif