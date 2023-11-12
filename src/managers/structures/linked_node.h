#ifndef LINKED_NODE

#include "space_block.h"

typedef struct linked_node *linked_node;
struct linked_node
{
    space_block block;
    linked_node next;
};

linked_node new_linked_node(space_block block, linked_node next);
#endif