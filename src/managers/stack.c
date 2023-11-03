#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "list.h"

stack_t *init_stack(size_t size, size_t stack_pointer)
{
    stack_t *new_stack = (stack_t *)malloc(sizeof(stack_t));
    new_stack->size_reserve = size;
    new_stack->sp = stack_pointer;
}

void Push(size_t value, sizeList_t *stack)
{
    push(stack, value);
}

size_t Pop(sizeList_t *stack)
{
    return pop(stack);
}