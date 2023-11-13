#ifndef SEG_STRUCTS_H
#define SEG_STRUCTS_H

#include <stdlib.h>
#include "../memory.h"
#include "../utils.h"
#include "free_list.h"

#define HEAP_SEG_SIZE 128
#define STACK_SEG_SIZE 64

typedef struct seg_proc {
  int pid;
  uint cod_base;
  uint cod_bounds;
  uint stack_base;  
  uint stack_pos;
  uint heap_base; 
  uint heap_pos;
} seg_proc_t;

typedef struct seg_node {
  seg_proc_t proc_mem_info;
  struct seg_node *next;
} seg_node_t;

void seg_node_clean(seg_node_t *head);
seg_node_t * seg_find_node(int pid,seg_node_t *head);

#endif