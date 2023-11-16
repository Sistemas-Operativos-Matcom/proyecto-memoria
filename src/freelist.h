#ifndef FREELIST_H
#define FREELIST_H

#include <malloc.h>
#include <stdio.h>
#include "utils.h"

#define bool int
#define true 1
#define false 0

typedef struct node
{
  struct node *next;
  bool used;
  size_t base;
  size_t bound;
} node;

typedef struct free_list
{
  node *root;
  node *last_node;
} free_list;

free_list *new_free_list(size_t espace);

int memory_malloc(free_list *list, size_t size, ptr_t *out);

int memory_free(free_list *list, ptr_t ptr);

bool memory_reduce(free_list *list);

void memory_expand(free_list *list, size_t size);

#endif