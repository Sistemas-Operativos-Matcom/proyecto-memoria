#include <stdio.h>
#include <stdlib.h>
#include "process_bnb.h"
process_bb *Init_p(int pid, size_t bound, size_t pos)
{
    process_bb *p = (process_bb *)malloc(sizeof(process_bb));
    p->pid = pid;
    p->base = bound * pos;
    p->bound = bound;
    p->memory = (size_t *)malloc(bound * sizeof(size_t));
    p->s = Init_s(bound);
    return p;
}
void Free_p(process_bb *p)
{
    Free_s(p->s);
    free(p->memory);
    free(p);
}
