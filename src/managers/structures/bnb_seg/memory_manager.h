#ifndef MEMORY_MANAGER

// #define STANDARD_SIZE 1000

#include "../../../utils.h"
#include "process_allocation.h"

typedef struct memory_manager *memory_manager;
struct memory_manager
{
    pcb current;
    pcb *pcbs;
    process_allocation *allocations;
    free_list space_list;
    size_t memory_size;
};

memory_manager new_memory_manager(size_t memory_size);
process_allocation find_allocation_mm(memory_manager manager, int pid);
bool change_process_mm(memory_manager manager, process_t process, bool on_bnb);
void remove_process_mm(memory_manager manager, process_t process);

#endif