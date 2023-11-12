#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
typedef struct Process
{
    int pid;
    size_t base;
    size_t bound;
    size_t *memory;
    stack *my_stack;
} process_bnb;

process_bnb *Init_proc_bnb(int pid, size_t bound, size_t pos);
void Free_p_bnb(process_bnb *p);
