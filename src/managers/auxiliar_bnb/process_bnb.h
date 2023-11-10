#include <stdio.h>
#include <stdlib.h>
#include "stack_bnb.h"
typedef struct Process
{
    int pid;
    size_t base;
    size_t bound;
    size_t *memory;
    stack *s;
} process_bb;

process_bb *Init_p(int pid, size_t bound, size_t pos);
void Free_p(process_bb *p);
