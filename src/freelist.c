#include <malloc.h>
#include "stdio.h"
#include <utils.h>
#include "freelist.h"

#define bool int
#define true 1
#define false 0

free_list *new_free_list(size_t espace)
{
  free_list *list = malloc(sizeof(free_list));
  list->count = 1;
  list->root = malloc(sizeof(node));
  list->root->base = 0;
  list->root->bound = espace;
  list->root->used = false;
  list->last_node = list->root;
  list->count = 1;
  list->free_espace = espace;

  return list;
}

bool memory_malloc(free_list *list, size_t size, ptr_t *out)
{
  node *actual_node = list->root;
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
      n->used = false;
      actual_node->next = n;
      actual_node->bound = size;
      if (n->next == NULL)
      {
        list->last_node = n;
      }
    }
    actual_node->used = true;
    out->addr = actual_node->base;
    out->size = actual_node->bound;
    return true;
  }
  else
    return false;
}

bool memory_free(free_list *list, ptr_t ptr)
{
  node *previous_node;
  node *actual_node = list->root;

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
    actual_node->used = false;
    if (previous_node != NULL && previous_node->used == false)
    {
      previous_node->next = actual_node->next;
      previous_node->bound = previous_node->bound + actual_node->bound;
      free(actual_node);
      actual_node = previous_node;
    }
    if ((actual_node->next != NULL && actual_node->next->used == false))
    {
      actual_node->next = actual_node->next->next;
      actual_node->bound = actual_node->bound + actual_node->next->bound;
      actual_node->used = false;
      free(actual_node->next);
      if (actual_node->next == NULL)
      {
        list->last_node = actual_node;
      }
    }
    return true;
  }
  else
    return false;
}

bool memory_reduce(free_list *list)
{
  if (!list->last_node->used && list->last_node->bound > 0)
  {
    list->last_node->bound = list->last_node->bound - 1;
    return true;
  }

  else
    return false;
}

void memory_expand(free_list *list)
{
  if (!list->last_node->used)
  {
    list->last_node->bound = list->last_node->bound + 1;
  }
  else
  {
    node *n = malloc(sizeof(node));
    n->base = list->last_node->base + list->last_node->bound;
    n->bound = 1;
    n->used = false;
    list->last_node->next = n;
    list->last_node = n;
  }
}