#ifndef ADDRESS_SPACE

#include "free_list.h"

// AS stands for Address Space
typedef struct address_space *address_space;
struct address_space
{
    addr_t code_start;
    addr_t code_end;
    addr_t heap_from;
    addr_t heap_to;
    addr_t stack_from;
    addr_t stack_to;
    free_list heap_list;
    space_block *heap_allocations;
    size_t stack_count;
    size_t size;
};

address_space new_address_space(size_t code_size, size_t heap_size, size_t stack_size);
bool is_valid_address_as(address_space space, addr_t address);
bool allocate_as(address_space space, size_t size, addr_t *pointer);
bool deallocate_as(address_space space, addr_t start, addr_t end);
bool push_as(address_space space, addr_t *address);
bool pop_as(address_space space, addr_t *address);
#endif