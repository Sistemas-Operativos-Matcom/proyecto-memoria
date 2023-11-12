#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

typedef struct Process
{
    int pid;
    int code_size;
    size_t *process_pages;
    size_t proc_pages_amount;
    size_t **pages_free_space;
    stack *my_stack;
} process_pag;

process_pag *Init_process_pag(int pid, size_t pages_size, size_t pag_table_c, int code_size);
void Free_p_pag(process_pag *p);
