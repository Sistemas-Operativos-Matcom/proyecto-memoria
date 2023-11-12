#include "linked_node.h"

linked_node new_linked_node(space_block block, linked_node next)
{
    linked_node _linked_node = (linked_node)malloc(sizeof(struct linked_node));

    _linked_node->block = block;
    _linked_node->next = next;

    return _linked_node;
}