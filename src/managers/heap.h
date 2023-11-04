#ifndef HEAP_H
#define HEAP_H

#include "list.h"
#include "../memory.h"
#include "../utils.h"

typedef struct heap
{
    sizeList_t *list;
    size_t start_virtual_pointer;
} heap_t;

heap_t *init_heap(process_t process);
#endif