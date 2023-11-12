#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
stack *Init_stack(int size)
{
    stack *my_stack = (stack *)malloc(sizeof(stack));
    my_stack->stack_mem = (size_t *)malloc(size * sizeof(size_t));
    my_stack->stack_size = size;
    my_stack->top = -1;
    return my_stack;
}
void Free_stack(stack *my_stack)
{
    free(my_stack->stack_mem);
    free(my_stack);
}
int Push_stack(stack *my_stack, size_t addr)
{
    if (my_stack->top == my_stack->stack_size - 1)
    {
        fprintf(stderr, "Stack overflow\n");
        return 1;
    }
    my_stack->top++;
    my_stack->stack_mem[my_stack->top] = addr;
    return 0;
}
size_t Pop_stack(stack *my_stack)
{
    if (my_stack->top == -1)
    {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    size_t value = my_stack->stack_mem[my_stack->top];
    my_stack->top--;
    return value;
}
