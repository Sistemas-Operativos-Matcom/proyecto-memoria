#include <stdio.h>
#include <stdlib.h>

typedef struct stack
{
    int stack_size;
    size_t *stack_mem;
    int top;
} stack;
stack *Init_stack(int size);

int Push_stack(stack *my_stack, size_t addr);
size_t Pop_stack(stack *my_stack);
void Free_stack(stack *my_stack);
