#include <stdio.h>
#include <stdlib.h>
#include "process_bnb.h"
process_bnb *Init_proc_bnb(int pid, size_t bound, size_t pos)
{
    process_bnb *p = (process_bnb *)malloc(sizeof(process_bnb));
    p->pid = pid;
    p->base = bound * pos;
    p->bound = bound;
    p->memory = (size_t *)malloc(bound * sizeof(size_t));
    p->my_stack = Init_stack(bound);
    return p;
}
void Free_p_bnb(process_bnb *p)
{
    Free_stack(p->my_stack);
    free(p->memory);
    free(p);
}
