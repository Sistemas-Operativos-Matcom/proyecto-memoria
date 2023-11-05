#ifndef HEAP_H
#define HEAP_H

#include "../memory.h"
#include "stack.h"


typedef struct Heap{
    int  *used_slots;
    size_t from_addr;
    size_t to_addr;
    int (*reserve)(size_t size, size_t stack_limit, struct Heap *heap);
} Heap_t;

Heap_t Heap_init(size_t from, int max_len);

#endif