#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "list.h"
#include <stddef.h>

stack_t *init_stack(int size, int program_size)
{
    stack_t *new_stack = malloc(sizeof(stack_t));
    new_stack->size_reserve = size == 0 ? 128 : size;
    new_stack->sp = size + program_size - 1;
    return new_stack;
}
