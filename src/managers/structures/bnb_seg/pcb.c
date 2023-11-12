#include "pcb.h"

pcb new_pcb(int pid, size_t code_size, addr_t heap_from, addr_t heap_to, addr_t stack_from, addr_t stack_to)
{
    pcb process = (pcb)malloc(sizeof(struct pcb));

    process->pid = pid;
    process->physical_heap_from = heap_from;
    process->physical_heap_to = heap_to;
    process->physical_stack_from = stack_from;
    process->physical_stack_to = stack_to;

    size_t heap_size = heap_to - heap_from + 1;
    size_t stack_size = stack_to - stack_from + 1;
    process->virtual_space = new_address_space(code_size, heap_size, stack_size);

    return process;
}

// from bnb/seg pcb
bool translate_virtual_to_physical_heap(pcb process, addr_t virtual_address, addr_t *physical_address)
{
    if (!is_valid_address_as(process->virtual_space, virtual_address))
        return FALSE;

    *physical_address = process->physical_heap_from + virtual_address;
    return TRUE;
}

// from bnb/seg pcb
bool translate_virtual_to_physical_stack(pcb process, addr_t virtual_address, addr_t *physical_address)
{
    long long gap = (long long)(process->virtual_space->size - 1) - (long long)virtual_address;
    size_t stack_size = process->virtual_space->stack_to - process->virtual_space->stack_from;

    if (gap < 0 || (size_t)gap >= stack_size || gap > process->virtual_space->stack_count)
        return FALSE;

    *physical_address = process->physical_stack_to - gap;
    return TRUE;
}

// bool translate_virtual_to_heap(pcb process, addr_t physical_address, addr_t *virtual_address)
// {
//     *virtual_address = physical_address - process->physical_heap_from;

//     if (!is_valid_address_as(process->virtual_space, *virtual_address))
//         return FALSE;

//     return TRUE;
// }