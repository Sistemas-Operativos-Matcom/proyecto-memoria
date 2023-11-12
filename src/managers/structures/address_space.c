#include "address_space.h"

address_space new_address_space(size_t code_size, size_t heap_size, size_t stack_size)
{
    address_space space = (address_space)malloc(sizeof(struct address_space));

    space->size = code_size + heap_size + stack_size;

    space->code_start = 0;
    space->code_end = code_size - 1;
    space->heap_from = code_size;
    space->heap_to = code_size + heap_size - 1;
    space->stack_from = code_size + heap_size;
    space->stack_to = code_size + heap_size + stack_size - 1;

    space->heap_list = new_free_list();
    initialize_free_list(space->heap_list, heap_size);

    space->heap_allocations = (space_block *)malloc(heap_size * sizeof(space_block));

    for (size_t i = 0; i < heap_size; i++)
        space->heap_allocations[i] = NULL;

    space->stack_count = 0;

    return space;
}

bool is_valid_allocation(address_space space, addr_t start, addr_t end)
{
    size_t heap_size = space->heap_to - space->heap_from + 1;

    for (size_t i = 0; i < heap_size; i++)
    {
        space_block allocation = space->heap_allocations[i];

        if (allocation != NULL && allocation->start == start && allocation->end == end)
            return TRUE;
    }

    return FALSE;
}

// Returns false if the allocation storage could not be made
bool add_allocation(address_space space, space_block block)
{
    size_t heap_size = space->heap_to - space->heap_from + 1;

    for (size_t i = 0; i < heap_size; i++)
    {
        if (space->heap_allocations[i] == NULL)
        {
            space->heap_allocations[i] = block;
            return TRUE;
        }
    }

    return FALSE;
}

bool remove_allocation(address_space space, addr_t start, addr_t end)
{
    size_t heap_size = space->heap_to - space->heap_from + 1;

    for (size_t i = 0; i < heap_size; i++)
    {
        space_block allocation = space->heap_allocations[i];

        if (allocation != NULL && allocation->start == start && allocation->end == end)
        {
            space->heap_allocations[i] = NULL;
            free(allocation);
            return TRUE;
        }
    }

    return FALSE;
}

// from address_space
bool is_valid_address_as(address_space space, addr_t address)
{
    size_t heap_size = space->heap_to - space->heap_from + 1;

    for (size_t i = 0; i < heap_size; i++)
    {
        space_block allocation = space->heap_allocations[i];

        if (allocation != NULL && allocation->start <= address && address < allocation->end)
            return TRUE;
    }

    return FALSE;
}

// from address space
// Returns NULL if there was no space available
bool allocate_as(address_space space, size_t size, addr_t *pointer)
{
    addr_t *bounds = get_space_free_list(space->heap_list, size, first_fit);

    if (bounds == NULL)
        return FALSE;

    // Store block in space->allocations
    space_block block = new_space_block(bounds[0], bounds[1]);
    if (!add_allocation(space, block))
        return FALSE;

    *pointer = bounds[0];

    return TRUE;
}

bool deallocate_as(address_space space, addr_t start, addr_t end)
{
    if (!is_valid_allocation(space, start, end))
        return FALSE;
    if (!remove_allocation(space, start, end))
        return FALSE;

    return_space_free_list(space->heap_list, start, end);

    return TRUE;
}

bool push_as(address_space space, addr_t *address)
{
    size_t stack_size = space->stack_to - space->stack_from + 1;
    if (space->stack_count >= stack_size)
        return FALSE; // Stack Overflow

    *address = space->stack_to - space->stack_count;
    space->stack_count++; // Pushing
    return TRUE;
}

bool pop_as(address_space space, addr_t *address)
{
    if (space->stack_count == 0)
        return FALSE; // Stack's empty

    space->stack_count--; // Popping
    *address = space->stack_to - space->stack_count;
    return TRUE;
}