#include "pag_structs.h"

#include "stdio.h"


void pag_clear_v(pag_t *pagref)
{
    if (pagref->virtual_mem != NULL) 
    {
        free(pagref->virtual_mem);
        pagref->virtual_mem = NULL;
    }
    if (pagref->process != NULL) {
        free(pagref->process);
        pagref->process = NULL;
    }
    if (pagref->PageFrameNum != NULL) {
        free(pagref->PageFrameNum);
        pagref->PageFrameNum = NULL;
    }
}

void pag_init_m(pag_t *pagref,size_t cantPF)
{
    pagref->process = (info_process_t *)malloc(cantPF * sizeof(info_process_t));
    pagref->PageFrameNum = (int *)malloc(cantPF * sizeof(int));
    pagref->virtual_mem = (int *)malloc(cantPF * sizeof(int));

    for (size_t i = 0; i < cantPF; i++) 
    {
        pagref->process[i].asig = 0;
        pagref->process[i].page_table = (size_t *)malloc(PAGES * sizeof(size_t));

        for (size_t j = 0; j < PAGES; j++) 
        {
            pagref->process[i].page_table[j] = -1;
        }

        pagref->process[i].user_pid = -1;
        pagref->process[i].heap = 0;
        pagref->process[i].stack = STACK_SIZE;
        pagref->virtual_mem[i] = -1;
        pagref->PageFrameNum[i] = i;
    }
}

size_t pag_find_freemem_a(pag_t *pagref,size_t cantPF)
{
    for (size_t i = 0; i < cantPF; i++)
    {
        if (pagref->virtual_mem[i] == -1) return i;
    }
    return -1;
}

size_t pag_find_pidproc_b(pag_t *pagref,size_t cantPF,int pid)
{
    for (size_t i = 0; i < cantPF; i++) 
    {
        if (pagref->virtual_mem[i] == pid) return i;
    }    
    return -1;
}

size_t pag_find_freeproc_c(pag_t *pagref,size_t cantPF)
{
    for (size_t i = 0; i < cantPF; i++) 
    {
        if (pagref->process[i].asig == 0) return i;
    }
    return -1;
}