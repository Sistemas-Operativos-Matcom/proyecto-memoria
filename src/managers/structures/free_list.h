#ifndef FREE_LIST

#include "bool.h"
#include "linked_node.h"

typedef struct free_list *free_list;
struct free_list
{
    linked_node head;
    linked_node tail;
    size_t count;
};

typedef addr_t *(*get_space_action)(free_list, size_t);

// General-linked-list method
free_list new_free_list();

// Fundamental free-list methods
void initialize_free_list(free_list list, size_t total_size);
addr_t *get_space_free_list(free_list list, size_t size, get_space_action get_space);
void return_space_free_list(free_list list, addr_t start, addr_t end);

// Auxiliary free-list method
addr_t *first_fit(free_list list, size_t size);

#endif