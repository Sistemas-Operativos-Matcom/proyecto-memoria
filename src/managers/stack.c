#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "list.h"
#include <stddef.h>

stack_t *init_stack(size_t size, size_t program_size)
{
    stack_t *new_stack = (stack_t *)malloc(sizeof(stack_t));
    new_stack->size_reserve = size == 0 ? 128 : size;
    new_stack->sp = size + program_size;
    return new_stack;
}

void Push(size_t value, sizeList_t *stack)
{
    push(stack, value);
}

size_t Pop(sizeList_t *stack)
{
    return pop(stack);
}