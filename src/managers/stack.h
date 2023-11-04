#ifndef STACK_H
#define STACK_H

#include "heap.h"
#include "stdio.h"
#include "../memory.h"
#include "../utils.h"


typedef struct Stack
{
    size_t from_addr;
    size_t to_addr;
    Heap_t heap;
    int len;
    int (*push)(byte *val, ptr_t out, struct Stack *stack);
    int (*pop)(byte *out, struct Stack *stack);
} Stack_t;

Stack_t Stack_init(size_t from, Heap_t heap);
#endif