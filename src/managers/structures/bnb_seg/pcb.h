#ifndef PCB

#include "address_space.h"

// PCB stands for Process Control Block
typedef struct pcb *pcb;
struct pcb
{
    int pid;
    // addr_t physical_code_start;
    // addr_t physical_code_end;
    addr_t physical_heap_from;
    addr_t physical_heap_to;
    addr_t physical_stack_from;
    addr_t physical_stack_to;
    address_space virtual_space;
};

bool translate_virtual_to_physical_heap(pcb process, addr_t virtual_address, addr_t *physical_address);
bool translate_virtual_to_physical_stack(pcb process, addr_t virtual_address, addr_t *physical_address);
#endif