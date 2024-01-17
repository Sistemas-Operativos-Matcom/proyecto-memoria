#include <malloc.h>
#include "stdio.h"
#include "utils.h"
#include "freelist.h"
#include "manager.h"
#include "memory.h"

free_list *new_free_list(size_t space)
{
  free_list *list = malloc(sizeof(free_list));
  list->head = malloc(sizeof(node));
  list->head->base = 0;
  list->head->bound = space;
  list->head->used = 0;
  list->head->next = NULL;
  list->last_node = list->head;

  return list;
}

int memory_malloc(free_list *list, size_t size, ptr_t *out)
{
  node *actual_node = list->head;
  while (actual_node != NULL)
  {
    if (!actual_node->used && actual_node->bound >= size)
      break;
    else
      actual_node = actual_node->next;
  }
  if (actual_node != NULL)
  {
    if (actual_node->bound > size)
    {
      node *n = malloc(sizeof(node));
      n->next = actual_node->next;
      n->base = actual_node->base + size;
      n->bound = actual_node->bound - size;
      n->used = 0;
      actual_node->next = n;
      actual_node->bound = size;
      if (n->next == NULL)
      {
        list->last_node = n;
      }
    }
    actual_node->used = 1;
    out->addr = actual_node->base;
    out->size = actual_node->bound;
    return 1;
  }
  else
    return 0;
}

int memory_free(free_list *list, ptr_t ptr)
{
  node *previous_node;
  node *actual_node = list->head;

  while (actual_node != NULL)
  {
    if (actual_node->used && actual_node->base == ptr.addr && actual_node->bound == ptr.size)
      break;
    else
    {
      previous_node = actual_node;
      actual_node = actual_node->next;
    }
  }

  if (actual_node != NULL)
  {
    actual_node->used = 0;
    if (previous_node != NULL && previous_node->used == 0)
    {
      previous_node->next = actual_node->next;
      previous_node->bound = previous_node->bound + actual_node->bound;
      free(actual_node);
      actual_node = previous_node;
    }
    if ((actual_node->next != NULL && actual_node->next->used == 0))
    {
      actual_node->next = actual_node->next->next;
      actual_node->bound = actual_node->bound + actual_node->next->bound;
      actual_node->used = 0;
      free(actual_node->next);
      if (actual_node->next == NULL)
      {
        list->last_node = actual_node;
      }
    }
    return 1;
  }
  else
    return 0;
}

int memory_reduce(free_list *list)
{
  if (!list->last_node->used && list->last_node->bound > 0)
  {
    list->last_node->bound = list->last_node->bound - 1;
    return 1;
  }

  else
    return 0;
}

void memory_expand(free_list *list, size_t size)
{
  if (!list->last_node->used)
  {
    list->last_node->bound = list->last_node->bound + size;
  }
  else
  {
    node *n = malloc(sizeof(node));
    n->base = list->last_node->base + list->last_node->bound;
    n->bound = size;
    n->used = 0;
    list->last_node->next = n;
    list->last_node = n;
  }
}