#include <stdio.h>
#include <stdlib.h>
#include "stack_pag.h"

typedef struct Process
{
    /*
     size_t *pag_code;
     size_t *pag_heap;
     size_t pag_heap_pos;
     size_t *pag_stack;
     size_t pag_stack_pos;
     */
    int pid;
    size_t *pag_process;
    size_t pag_process_c;
    size_t *pag_process_free;
    stack *s;
} process_pag;

process_pag *Init_p_pag(int pid, size_t pag_code, size_t pag_table_c);
void Free_p_pag(process_pag *p);
