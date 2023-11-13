#ifndef PAG_HEAP_H
#define PAG_HEAP_H

#include "stdio.h"
#include "../memory.h"
#include "../utils.h"

typedef struct Pag_Heap
{
    int* pages_base;
    int* pages_bound;
    int* len;
    int (*pag_reserve)(size_t size, addr_t page_bound, struct Pag_Heap *heap);
    int (*pag_load)(addr_t addr, byte *out, struct Pag_Heap *heap);
    int (*pag_store)(addr_t addr, byte val, struct Pag_Heap *heap);
} Pag_Heap_t;

Pag_Heap_t Pag_Heap_init(int base);
#endif