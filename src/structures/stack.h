#ifndef STACK_H
#define STACK_H

#include "../memory.h"
#include "../utils.h"
#include "heap.h"
#include <stdio.h>


typedef struct Stack
{
    size_t from_addr;
    size_t to_addr;
    int len;
    int (*push)(byte val, ptr_t out, struct Stack *stack, Heap_t *process_heap);
    int (*pop)(byte *out, struct Stack *stack);
} Stack_t;

int push(byte val, ptr_t out, Stack_t *stack, Heap_t *process_heap)
{
    if(stack->to_addr+sizeof(size_t) == process_heap->to_addr) return 1;

    m_write(stack->to_addr, val);
    stack->to_addr+=sizeof(size_t);
    stack->len++;
    return 0;
}
int pop(byte *out, Stack_t *stack)
{
    return 0;
}

Stack_t Stack_init(size_t base){

    Stack_t stack;
    stack.from_addr = base;
    stack.to_addr = base+sizeof(size_t);
    stack.len = 0;
    stack.push = push;
    stack.pop = pop;
    
    return stack;
}

#endif