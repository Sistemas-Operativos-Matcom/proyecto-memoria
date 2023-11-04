#include "process_allocation.h"

process_allocation new_process_allocation(int pid, size_t max_amount)
{
    process_allocation allocation = (process_allocation)malloc(sizeof(struct process_allocation));

    allocation->pid = pid;
    allocation->max_amount = max_amount;
    allocation->blocks = (space_block *)malloc(max_amount * sizeof(space_block));

    for (int i = 0; i < max_amount; i++)
        allocation->blocks[i] = NULL;

    return allocation;
}