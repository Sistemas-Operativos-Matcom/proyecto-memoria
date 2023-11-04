#ifndef PROCESS_ALLOCATION

#include "../space_block.h"

typedef struct process_allocation *process_allocation;
struct process_allocation
{
    int pid;
    space_block *blocks;
    size_t max_amount;
};

process_allocation new_process_allocation(int pid, size_t max_amount);
#endif