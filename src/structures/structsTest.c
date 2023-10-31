#include "../memory.h"
#include "../utils.h"
#include "stack.h"
#include "heap.h"
#include "stdio.h"


int main()
{
    Stack_t stack = Stack_init(1);
    Heap_t heap;
    byte val;
    ptr_t out;
    stack.push(val, out, &stack, &heap);
    stack.push(val, out, &stack, &heap);

    return 0;
}