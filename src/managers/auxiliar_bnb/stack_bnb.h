#include <stdio.h>
#include <stdlib.h>

typedef struct Stack
{
    int stack_size;
    size_t *stack_s;
    int top;
} stack;
stack *Init_s(int size);
int Push_s(stack *s, size_t value);
size_t Pop_s(stack *s);
void Free_s(stack *s);