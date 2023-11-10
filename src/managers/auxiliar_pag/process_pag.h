#include <stdio.h>
#include <stdlib.h>
#include "stack_pag.h"

typedef struct Process
{
    int pid;
    size_t *pag_process;
    size_t pag_process_c;
    size_t **pag_process_free;
    stack *s;
} process_pag;

process_pag *Init_p_pag(int pid, size_t pag_size, size_t pag_table_c);
void Free_p_pag(process_pag *p);
