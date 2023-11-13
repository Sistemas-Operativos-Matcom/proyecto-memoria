#ifndef PAG_STRUCTS_H
#define PAG_STRUCTS_H

#include <stdlib.h>
#include "../memory.h"
#include "../utils.h"

#define PAGES 4
#define BLOCK_SIZE 256
#define CODE_SIZE 1
#define STACK_SIZE (PAGES * BLOCK_SIZE)
#define KB(size) size / BLOCK_SIZE

typedef struct info_process 
{
    byte asig;
    int user_pid;
    size_t *page_table;
    size_t heap;
    size_t stack;
} info_process_t;

typedef struct pag
{
    int *virtual_mem;
    info_process_t *process;
    int *PageFrameNum;
    int pid_proc;
    int index_proc;
}pag_t;

void pag_clear_v(pag_t *pagref);
void pag_init_m(pag_t *pagref,size_t cantPF);
size_t pag_find_freemem_a(pag_t *pagref,size_t cantPF);
size_t pag_find_pidproc_b(pag_t *pagref,size_t cantPF,int pid);
size_t pag_find_freeproc_c(pag_t *pagref,size_t cantPF);




#endif