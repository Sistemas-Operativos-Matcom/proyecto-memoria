#ifndef PAG_HEAP_H
#define PAG_HEAP_H

#include "stdio.h"
#include "../memory.h"
#include "../utils.h"

typedef struct Pag_Heap
{
    int** used_slots;
    int* pages_base;
    int* pages_bound;
    int* len;
    int (*pag_reserve)(size_t size, struct Pag_Heap *heap);
    int (*pag_load)(addr_t addr, byte *out, struct Pag_Heap *heap);
    int (*pag_store)(addr_t addr, byte val, struct Pag_Heap *heap);
} Pag_Heap_t;

Pag_Heap_t Pag_Heap_init(int base, int size);


typedef struct Pag_Stack
{
    int* pages_base;
    int* pages_bound;
    int* SP;
    size_t base;
    size_t top;
    int (*pag_push)(byte *val, ptr_t *out, struct Pag_Stack *stack);
    int (*pag_pop)(byte *out, struct Pag_Stack *stack);
} Pag_Stack_t;

Pag_Stack_t Pag_Stack_init(int from, int size);

#endif