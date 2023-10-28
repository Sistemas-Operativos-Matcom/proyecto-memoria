#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
stack *Init_s(int size)
{
    stack *s = (stack *)malloc(sizeof(stack));
    s->stack_s = (size_t *)malloc(size * sizeof(size_t));
    s->stack_size = size;
    s->top = -1;
    return s;
}
void Free_s(stack *s)
{
    free(s->stack_s);
    free(s);
}
int Push_s(stack *s, size_t value)
{
    if (s->top == s->stack_size - 1)
    {
        fprintf(stderr, "Stack overflow\n");
        return 1;
    }
    s->top++;
    s->stack_s[s->top] = value;
    return 0;
}

size_t Pop_s(stack *s)
{
    if (s->top == -1)
    {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    size_t value = s->stack_s[s->top];
    s->top--;
    return value;
}
