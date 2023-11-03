#ifndef PROCESS_H
#define PROCESS_H

#include "../memory.h"
#include "../utils.h"
#include "list.h"
#include "stack.h"

typedef struct process_pag
{
    process_t process;
    sizeList_t *page_frames_indexed_by_virtual_pages;
    virtual_mem_t *v_memory;
} process_pag_t;

// Constructor for process_pag_t
process_pag_t *init_process_pag(process_t process);

typedef struct virtual_mem
{

    sizeList_t *heap;
    stack_t *stack;
} virtual_mem_t;

#endif