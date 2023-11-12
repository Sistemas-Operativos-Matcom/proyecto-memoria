#ifndef HEAP_H
#define HEAP_H

#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include <stddef.h>

typedef struct heap
{
    sizeList_t *list;
    int start_virtual_pointer;
    int end_virtual_pointer;
} heap_t;

heap_t *init_heap();
#endif