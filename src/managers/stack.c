#include "../memory.h"
#include "../utils.h"
#include "stack.h"


int push(byte *val, ptr_t out, Stack_t *stack)
{
    // if(stack->to_addr+sizeof(size_t) == stack->heap.to_addr) return 1;

    // m_write(stack->to_addr, *val);
    // stack->to_addr+=sizeof(size_t);
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
    stack.from_addr = from;
    stack.to_addr = from;
    stack.push = push;
    stack.pop = pop;
    
    return stack;
}