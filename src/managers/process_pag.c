#include "../memory.h"
#include "../utils.h"
#include "list.h"
#include "process_pag.h"
#include "pag_manager.h"
#include "pag_manager.c"

process_pag_t *init_process_pag(process_t process)
{
    process_pag_t *new_process_pag = (process_pag_t *)malloc(sizeof(process_pag_t));

    virtual_mem_t *v_memory = init_virtual_mem(process);
    sizeList_t *virtual_pages = init();
    // virtual_pages->size = ((m_size() * 2 / 10) / (PAGE_SIZE)); // cantidad de pag = 20% de m_size
    size_t amount_of_necessary_pages = 0;
    for (size_t i = 0; i < process.program->size; i++)
    {
        if (i % PAGE_SIZE == 0)
            amount_of_necessary_pages++;
    }

    virtual_pages->size = amount_of_necessary_pages + 1;
    new_process_pag->process = process;
    new_process_pag->page_frames_indexed_by_virtual_pages = virtual_pages;
    new_process_pag->v_memory = v_memory;
    return new_process_pag;
}

// Constructor for virtual_mem_t
virtual_mem_t *init_virtual_mem(process_t process)
{
    virtual_mem_t *new_virtual_mem = (virtual_mem_t *)malloc(sizeof(virtual_mem_t));
    heap_t *new_heap = init_heap(process);
    new_heap->list->size = PAGE_SIZE;

    new_virtual_mem->heap = new_heap;
    stack_t *stack = init_stack(((m_size() * 2 / 10) / (PAGE_SIZE)) * 3 / 10 / PAGE_SIZE, process.program->size); // tamano de stack = 30% de cantidad de pag
    new_virtual_mem->stack = stack;
    return new_virtual_mem;
}
