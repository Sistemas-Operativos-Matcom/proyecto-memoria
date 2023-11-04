#ifndef STACK_H
#define STACK_H

#include "list.h"
#include "../memory.h"
#include "../utils.h"

typedef struct stack
{
    size_t size_reserve;
    size_t sp;
} stack_t;

stack_t *init_stack(size_t size, size_t program_size);

void Push(size_t value, sizeList_t *stack);

size_t Pop(sizeList_t *stack);

#endif