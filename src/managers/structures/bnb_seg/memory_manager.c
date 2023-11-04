#include <stdio.h>

#include "memory_manager.h"

// from bnb/seg memory manager
process_allocation find_allocation_mm(memory_manager manager, int pid)
{
    for (size_t i = 0; i < manager->memory_size; i++)
    {
        process_allocation allocation = manager->allocations[i];
        if (allocation != NULL && allocation->pid == pid)
            return allocation;
    }
    return NULL;
}

void add_process_allocation(memory_manager manager, int pid, addr_t from, addr_t to)
{
    // Exception to convention in this case
    space_block block = new_space_block(from, to);
    process_allocation allocation = find_allocation_mm(manager, pid);

    if (allocation == NULL)
    {
        allocation = new_process_allocation(pid, 100);
        for (size_t i = 0; i < manager->memory_size; i++)
        {
            if (manager->allocations[i] == NULL)
            {
                manager->allocations[i] = allocation;
                break;
            }
        }
    }

    for (size_t i = 0; i < allocation->max_amount; i++)
    {
        if (allocation->blocks[i] == NULL)
        {
            allocation->blocks[i] = block;
            break;
        }
    }
}

// from memory bnb/seg memory manager
bool remove_process_allocation(memory_manager manager, int pid)
{
    for (size_t i = 0; i < manager->memory_size; i++)
    {
        process_allocation allocation = manager->allocations[i];
        if (allocation != NULL && allocation->pid == pid)
        {
            manager->allocations[i] = NULL;
            free(allocation);
            return TRUE;
        }
    }
    return FALSE;
}

memory_manager new_memory_manager(size_t memory_size)
{
    memory_manager manager = (memory_manager)malloc(sizeof(struct memory_manager));
    manager->memory_size = memory_size;

    manager->current = NULL;
    manager->space_list = new_free_list();
    initialize_free_list(manager->space_list, memory_size);

    manager->pcbs = (pcb *)malloc(memory_size * sizeof(pcb));
    manager->allocations = (process_allocation *)malloc(memory_size * sizeof(process_allocation));

    for (size_t i = 0; i < memory_size; i++)
    {
        manager->pcbs[i] = NULL;
        manager->allocations[i] = NULL;
    }

    return manager;
}

pcb find_process(memory_manager manager, int pid)
{
    for (size_t i = 0; i < manager->memory_size; i++)
    {
        pcb _pcb = manager->pcbs[i];
        if (_pcb != NULL && _pcb->pid == pid)
            return _pcb;
    }

    return NULL;
}

bool add_process(memory_manager manager, pcb _pcb)
{
    for (size_t i = 0; i < manager->memory_size; i++)
    {
        if (manager->pcbs[i] == NULL)
        {
            manager->pcbs[i] = _pcb;
            return TRUE;
        }
    }

    return FALSE;
}

bool remove_process(memory_manager manager, int pid)
{
    for (size_t i = 0; i < manager->memory_size; i++)
    {
        pcb _pcb = manager->pcbs[i];
        if (_pcb != NULL && _pcb->pid == pid)
        {
            manager->pcbs[i] = NULL;
            free(_pcb);
            return TRUE;
        }
    }

    return FALSE;
}

pcb create_process_bnb(memory_manager manager, int pid, size_t code_size)
{
    size_t process_size = code_size + STANDARD_SIZE;
    addr_t *bounds = get_space_free_list(manager->space_list, process_size, first_fit);

    add_process_allocation(manager, pid, bounds[0], bounds[1] - 1);

    addr_t heap_from = bounds[0] + code_size;
    addr_t heap_to = heap_from - 1 + STANDARD_SIZE / 2;
    addr_t stack_from = heap_from + STANDARD_SIZE / 2;
    addr_t stack_to = bounds[1] - 1;

    return new_pcb(pid, code_size, heap_from, heap_to, stack_from, stack_to);
}

pcb create_process_seg(memory_manager manager, int pid, size_t code_size)
{
    addr_t *code_bounds = get_space_free_list(manager->space_list, code_size, first_fit);
    addr_t *heap_bounds = get_space_free_list(manager->space_list, STANDARD_SIZE / 2, first_fit);
    addr_t *stack_bounds = get_space_free_list(manager->space_list, STANDARD_SIZE / 2, first_fit);

    add_process_allocation(manager, pid, code_bounds[0], code_bounds[1] - 1);
    add_process_allocation(manager, pid, heap_bounds[0], heap_bounds[1] - 1);
    add_process_allocation(manager, pid, stack_bounds[0], stack_bounds[1] - 1);

    printf("code_l: %lld \n", code_bounds[0]);
    printf("code_u: %lld \n", code_bounds[1] - 1);
    printf("heap_l: %lld \n", heap_bounds[0]);
    printf("heap_u: %lld \n", heap_bounds[1] - 1);
    printf("stack_l: %lld \n", stack_bounds[0]);
    printf("stack_u: %lld \n", stack_bounds[1] - 1);

    return new_pcb(pid, code_size, heap_bounds[0], heap_bounds[1] - 1, stack_bounds[0], stack_bounds[1] - 1);
}

// from memory bnb/seg memory manager
bool change_process_mm(memory_manager manager, process_t process, bool on_bnb)
{
    pcb _pcb = find_process(manager, process.pid);

    printf("Cuba: %d \n", process.pid);
    if (_pcb == NULL)
    {
        if (on_bnb)
            _pcb = create_process_bnb(manager, process.pid, process.program->size);
        else
            _pcb = create_process_seg(manager, process.pid, process.program->size);

        add_process(manager, _pcb);

        manager->current = _pcb;
        return TRUE;
    }

    manager->current = _pcb;
    return FALSE;
}

// from memory bnb/seg memory manager
void remove_process_mm(memory_manager manager, process_t process)
{
    if (!remove_process(manager, process.pid))
        fprintf(stderr, "Process was not found"), exit(1);
}