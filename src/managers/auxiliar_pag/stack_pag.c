#include <stdio.h>
#include <stdlib.h>
#include "stack_pag.h"
stack *Init_s_pag(int size)
{
    stack *s = (stack *)malloc(sizeof(stack));
    s->stack_s = (dupla **)malloc(size * sizeof(struct Dupla *));
    s->stack_size = size;
    s->top = -1;
    return s;
}
void Free_s_pag(stack *s)
{
    for (int i = 0; i <= s->top; i++)
        free(s->stack_s[i]);

    free(s->stack_s);
    free(s);
}
int Push_s_pag(stack *s, size_t pag, size_t pos_pag)
{
    // printf("size pila: %u\n top pila: %i", s->stack_size, s->top);
    if (s->top == s->stack_size - 1)
    {
        fprintf(stderr, "Stack overflow este\n");
        return 1;
    }
    s->top++;
    s->stack_s[s->top] = (dupla *)malloc(sizeof(dupla));
    s->stack_s[s->top]->pag = pag;
    s->stack_s[s->top]->pos_pag = pos_pag;
    return 0;
}
/*
size_t Pop_s_pag(stack *s)
{
    if (s->top == -1)
    {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    size_t value = s->stack_s[s->top];
    s->top--;
    return value;
} */
dupla *Pop_s_pag(stack *s)
{
    if (s->top == -1)
    {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    dupla *value = s->stack_s[s->top];
    // printf("top: %u value: %zu\n", s->top, s->stack_s[s->top]->pos_pag);
    s->top--;
    return value;
}
