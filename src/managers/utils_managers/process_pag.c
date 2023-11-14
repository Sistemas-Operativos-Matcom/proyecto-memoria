#include <stdio.h>
#include <stdlib.h>
#include "process_pag.h"

process_pag *Init_process_pag(int pid, size_t pages_size, size_t frames_amount, int code_Size)
{
    process_pag *p = (process_pag *)malloc(sizeof(process_pag));
    p->pid = pid;
    p->code_size = code_Size;
    p->process_pages = (size_t *)malloc(frames_amount * sizeof(size_t));
    p->proc_pages_amount = 0;
    p->pages_free_space = (size_t **)malloc(frames_amount * sizeof(size_t *));
    for (size_t i = 0; i < frames_amount; i++)
        p->pages_free_space[i] = (size_t *)malloc(pages_size * sizeof(size_t));
    p->my_stack = Init_stack(frames_amount);
    return p;
}
void Free_p_pag(process_pag *proc)
{
    Free_stack(proc->my_stack);
    free(proc->pages_free_space);
    free(proc->process_pages);
    free(proc);
}
