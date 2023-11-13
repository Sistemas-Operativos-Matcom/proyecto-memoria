#ifndef HEAP_H
#define HEAP_H

#include "../memory.h"
#include "stack.h"


typedef struct Heap{
    int  *used_slots;
    addr_t from_addr;
    addr_t to_addr;
    int (*reserve)(size_t size, addr_t stack_limit, struct Heap *heap);
    int (*load)(addr_t addr, byte *out, struct Heap *heap);
    int (*store)(addr_t addr, byte val, struct Heap *heap);
} Heap_t;

Heap_t Heap_init(size_t from, int max_len);

#endif