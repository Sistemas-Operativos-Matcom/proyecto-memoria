#include <stdio.h>
#include <stdlib.h>
#include "process_pag.h"

process_pag *Init_p_pag(int pid, size_t pag_code, size_t pag_table_c)
{
    process_pag *p = (process_pag *)malloc(sizeof(process_pag));
    p->pid = pid; /*
     p->pag_code = (size_t *)malloc(pag_code * sizeof(size_t));
     p->pag_heap = (size_t *)malloc(pag_table_c * sizeof(size_t));
     p->pag_stack = (size_t *)malloc(pag_table_c * sizeof(size_t)); */
    p->pag_process = (size_t *)calloc(pag_table_c, sizeof(size_t));
    p->pag_process_c = 0;
    p->pag_process_free = (size_t *)calloc(pag_table_c, sizeof(size_t));

    p->s = Init_s_pag(pag_table_c);
    return p;
}
void Free_p_pag(process_pag *p)
{
    Free_s_pag(p->s); /*
     free(p->pag_code);
     free(p->pag_heap);
     free(p->pag_stack); */
    free(p->pag_process);
    free(p);
}
