#include "../memory.h"
#include "../utils.h"
#include "stack.h"


int push(byte *val, ptr_t *out, Stack_t *stack, addr_t limit)
{
    addr_t new_top = stack->top+(addr_t)1;
    // Checking push can be made
    if(new_top == limit) return 1;

    // Writing val in stack
    m_write(new_top, *val);
    stack->top = new_top;
    out->addr = new_top;
    out->size = sizeof(byte);
    return 0;
}

int pop(byte *out, Stack_t *stack)
{   
    // Checking pop can be made
    if(stack->top == stack->base) return 1;
    
    // Reading val from stack
    *out = m_read(stack->top);
    stack->top = stack->top - (addr_t)1;
    return 0;
}

Stack_t Stack_init(size_t from){

    Stack_t stack;
    stack.base = from;
    stack.top = from;
    stack.push = push;
    stack.pop = pop;
    
    return stack;
}