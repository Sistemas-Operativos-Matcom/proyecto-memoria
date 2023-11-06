#include "../memory.h"
#include "../utils.h"
#include "stack.h"


int push(byte *val, ptr_t *out, Stack_t *stack, addr_t limit)
{
    addr_t new_top = stack->top;
    // Checking push can be made
    if(new_top+(addr_t)1 == limit) return 1;

    // Writing val in stack
    m_write(new_top, *val);
    stack->top = new_top;
    out->addr = new_top;
    out->size = sizeof(byte);
    return 0;
}

int pop(byte *out, Stack_t *stack)
{   
    // *out = m_read(stack->to_addr);
    // stack->to_addr-=sizeof(size_t);
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