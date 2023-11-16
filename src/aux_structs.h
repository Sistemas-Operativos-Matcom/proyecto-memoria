#ifndef aux_structs_H
#define aux_structs_H

#include <malloc.h>
#include <stdio.h>
#include "utils.h"

typedef struct space
{
    addr_t address;
    addr_t size;
    char used;

    space_t *prev;
    space_t *next;
}space_t;

typedef struct free_list
{
  space_t *head;
  space_t *foot;
} free_list;

free_list *new_free_list(space_t space);

char memory_malloc(free_list *list, size_t size, ptr_t *out);

void memory_free(free_list *list, ptr_t ptr);


#endif