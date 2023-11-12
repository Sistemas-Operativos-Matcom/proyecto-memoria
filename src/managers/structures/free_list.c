#include <stdlib.h>
#include <stdio.h>

#include "free_list.h"

typedef void (*sort_space_blocks_action)(space_block *, addr_t);

// Auxiliary free-list methods
void coalesce(free_list list, sort_space_blocks_action sort_blocks);
void sort_space_blocks(space_block *blocks, size_t length);

// General-linked-list methods
linked_node search_bounds_free_list(free_list list, addr_t start, addr_t end, addr_t *node_index);
linked_node search_index_free_list(free_list list, addr_t index);
bool insert_bounds_free_list(free_list list, addr_t start, addr_t end);
bool insert_block_free_list(free_list list, space_block block);
bool remove_bounds_free_list(free_list list, addr_t start, addr_t end);
bool remove_index_free_list(free_list list, addr_t index);

// Auxiliary general-linked-list methods
void validate_bounds(addr_t, addr_t);
bool are_bounds_valid_free_list(free_list list, addr_t start, addr_t end);

free_list new_free_list()
{
    free_list _free_list = (free_list)malloc(sizeof(struct free_list));

    _free_list->head = NULL;
    _free_list->tail = NULL;
    _free_list->count = 0;

    return _free_list;
}

bool are_bounds_valid_free_list(free_list list, addr_t start, addr_t end)
{
    validate_bounds(start, end);

    linked_node current_node = list->head;

    // Traversing the free list
    while (current_node != NULL)
    {
        space_block current_block = current_node->block;

        bool matched = current_block->start == start && current_block->end == end;
        bool is_start_contained = current_block->start < start && start < current_block->end;
        bool is_end_contained = current_block->end < start && end < current_block->end;

        // Verifying block
        if (matched || is_start_contained || is_end_contained)
            return FALSE;

        // Moving forward
        current_node = current_node->next;
    }

    return TRUE;
}

linked_node search_bounds_free_list(free_list list, addr_t start, addr_t end, addr_t *node_index)
{
    validate_bounds(start, end);

    linked_node current_node = list->head;

    *node_index = 0;
    // Traversing the free list
    while (current_node != NULL)
    {
        space_block current_block = current_node->block;

        // Verifying block
        if (current_block->start == start && current_block->end == end)
            return current_node;

        // Moving forward
        current_node = current_node->next;
        *node_index += 1;
    }

    return NULL;
}

linked_node search_index_free_list(free_list list, addr_t index)
{
    if (index < 0 || index >= list->count)
        fprintf(stderr, "Index was outside of the bounds of the list"), exit(1);

    linked_node current_node = list->head;
    int i = 0;
    // Traversing the free list
    while (current_node != NULL)
    {
        if (i == index)
            return current_node;

        // Moving forward
        current_node = current_node->next;
        i++;
    }

    return NULL;
}

bool insert_bounds_free_list(free_list list, addr_t start, addr_t end)
{
    if (!are_bounds_valid_free_list(list, start, end))
        return FALSE;

    if (list->count == 0)
    {
        space_block block = new_space_block(start, end);
        list->head = list->tail = new_linked_node(block, NULL);
    }
    else
    {
        space_block block = new_space_block(start, end);
        linked_node tail = list->tail;
        tail->next = new_linked_node(block, NULL);
        list->tail = tail->next;
    }

    list->count++;
    return TRUE;
}

bool remove_bounds_free_list(free_list list, addr_t start, addr_t end)
{
    addr_t node_index;
    linked_node node = search_bounds_free_list(list, start, end, &node_index);

    if (node == NULL)
        return FALSE;

    linked_node previous_node;
    if (node_index == 0)
    {
        if (node->next != NULL)
            list->head = node->next;
        else
            list->head = list->tail = NULL;
    }
    else
    {
        previous_node = search_index_free_list(list, node_index - 1);
        previous_node->next = node->next;
        if (node->next == NULL)
            list->tail = previous_node;
    }

    list->count--;
    free(node);
    return TRUE;
}

bool remove_index_free_list(free_list list, addr_t index)
{
    linked_node node = search_index_free_list(list, index);

    if (node == NULL)
        return FALSE;

    if (index == 0)
    {
        if (node->next != NULL)
            list->head = node->next;
        else
            list->head = list->tail = NULL;
    }
    else
    {
        linked_node previous_node = search_index_free_list(list, index - 1);
        previous_node->next = node->next;
    }

    list->count--;
    free(node);
    return TRUE;
}

void sort_space_blocks(space_block *blocks, size_t length)
{
    for (size_t i = 0; i < length - 1; i++)
    {
        space_block smallest = blocks[i];
        for (size_t j = i + 1; j < length; j++)
        {
            space_block temp;
            if (blocks[j]->start < smallest->start)
            {
                smallest = blocks[j];

                // / Swapping values
                temp = blocks[i];
                blocks[i] = smallest;
                blocks[j] = temp;
            }
        }
    }
}

bool insert_block_free_list(free_list list, space_block block)
{
    if (!are_bounds_valid_free_list(list, block->start, block->end))
        return FALSE;

    if (list->count == 0)
    {
        list->head = list->tail = new_linked_node(block, NULL);
    }
    else
    {
        linked_node tail = list->tail;
        tail->next = new_linked_node(block, NULL);
        list->tail = tail->next;
    }

    list->count++;
    return TRUE;
}

addr_t *first_fit(free_list list, size_t size)
{
    addr_t *bounds = (addr_t *)malloc(2 * sizeof(addr_t));
    linked_node node = list->head;

    while (node != NULL)
    {
        space_block block = node->block;
        if (block->size >= size)
        {
            bounds[0] = block->start;
            bounds[1] = block->end;
            return bounds;
        }

        node = node->next;
    }

    return NULL;
}

void coalesce(free_list list, sort_space_blocks_action sort_blocks)
{
    const size_t COUNT = list->count;
    space_block blocks[COUNT];

    // Storing all blocks
    linked_node node = list->head;
    int i = 0;
    while (node != NULL)
    {
        blocks[i++] = node->block;
        linked_node temp = node;
        node = node->next;
        free(temp);
    }

    // Reseting free list
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    // Sorting block list according to their start bound
    sort_blocks(blocks, COUNT);

    // Colescing blocks
    for (int i = 0; i < COUNT - 1; i++)
    {
        space_block first_block = blocks[i];
        space_block second_block = blocks[i + 1];

        if (first_block->end == second_block->start)
        {
            blocks[i] = NULL;
            blocks[i + 1] = new_space_block(first_block->start, second_block->end);
            free(first_block);
            free(second_block);
        }
    }

    for (int i = 0; i < COUNT; i++)
        if (blocks[i] != NULL)
            insert_block_free_list(list, blocks[i]);
}

void initialize_free_list(free_list list, size_t total_size)
{
    space_block block = new_space_block(0, total_size);
    insert_block_free_list(list, block);
}

addr_t *get_space_free_list(free_list list, size_t size, get_space_action get_space)
{
    addr_t *bounds = get_space(list, size);

    if(bounds == NULL)
        return NULL;

    bool matched_size = (bounds[1] - bounds[0]) == size;

    addr_t *to_deliver = (addr_t *)malloc(2 * sizeof(addr_t));
    to_deliver[0] = bounds[0], to_deliver[1] = bounds[0] + size;

    addr_t to_store_back[2];
    if (!matched_size)
        to_store_back[0] = bounds[0] + size, to_store_back[1] = bounds[1];

    remove_bounds_free_list(list, bounds[0], bounds[1]);

    if (!matched_size)
        if (!insert_bounds_free_list(list, to_store_back[0], to_store_back[1]))
            fprintf(stderr, "Invalid bounds"), exit(1);

    free(bounds);
    return to_deliver;
}

void return_space_free_list(free_list list, addr_t start, addr_t end)
{
    if (!insert_bounds_free_list(list, start, end))
        fprintf(stderr, "Invalid bounds"), exit(1);

    coalesce(list, sort_space_blocks);
}