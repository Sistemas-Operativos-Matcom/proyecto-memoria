#ifndef FREELIST_H
#define FREELIST_H

#include <malloc.h>
#include <stdio.h>
#include "manager.h"
#include "memory.h"
#include "utils.h"


typedef struct node
{
  struct node *next;
  int used;
  size_t base;
  size_t bound;
} node;

typedef struct free_list
{
  node *head;
  node *last_node;
} free_list;

free_list *new_free_list(size_t espace);

int memory_malloc(free_list *list, size_t size, ptr_t *out);

int memory_free(free_list *list, ptr_t ptr);

int memory_reduce(free_list *list);

void memory_expand(free_list *list, size_t size);

#endif
