#include "../memory.h"
#include "../utils.h"
#include "list.h"
#include "process_pag.h"
#include "pag_manager.h"
#include "pag_manager.c"

process_pag_t *init_process_pag(process_t process)
{
    process_pag_t *new_process_pag = (process_pag_t *)malloc(sizeof(process_pag_t));

    virtual_mem_t *v_memory = init_virtual_mem();
    sizeList_t *page_frames = init();
    new_process_pag->process = process;
    new_process_pag->page_frames_indexed_by_virtual_pages = page_frames;
    new_process_pag->v_memory = v_memory;
    return new_process_pag;
}

// Constructor for virtual_mem_t
virtual_mem_t *init_virtual_mem(sizeList_t *heap, stack_t *stack)
{
    virtual_mem_t *new_virtual_mem = (virtual_mem_t *)malloc(sizeof(virtual_mem_t));
    new_virtual_mem->heap = heap;
    new_virtual_mem->stack = stack;
    return new_virtual_mem;
}
