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
    sizeList_t *new_pages_table = init();

    // size_t amount_of_necessary_pages = 0;
    // for (size_t i = 0; i < process.program->size; i++)
    // {
    //     if (i % PAGE_SIZE == 0)
    //         amount_of_necessary_pages++;
    // }

    // new_pages_table->size = amount_of_necessary_pages + 1;

    new_process_pag->process = process;
    new_process_pag->pages_table = new_pages_table;
    new_process_pag->v_memory = v_memory;
    return new_process_pag;
}

// Constructor for virtual_mem_t
virtual_mem_t *init_virtual_mem(process_t process)
{
    virtual_mem_t *new_virtual_mem = (virtual_mem_t *)malloc(sizeof(virtual_mem_t));
    heap_t *new_heap = init_heap();
    new_heap->list->size = PAGE_SIZE;

    stack_t *stack = init_stack(m_size() / 10, process.program->size); // tamano de stack = 10% de tamano de pag frames

    new_virtual_mem->stack = stack;

    new_heap->start_virtual_pointer = stack->size_reserve + process.program->size;
    new_heap->end_virtual_pointer = new_heap->start_virtual_pointer;

    new_virtual_mem->heap = new_heap;
    return new_virtual_mem;
}
