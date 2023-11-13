#ifndef BNB_STRUCTS_H
#define BNB_STRUCTS_H

#include <stdlib.h>
#include "../memory.h"
#include "../utils.h"

#define BLOCK_SIZE 512
#define STACK_BLOCK_SIZE 128

typedef struct bnb_process_block {
  int pid;
  uint base;  
  uint bounds;
  uint cod_bounds;
  uint stack_pos;
  uint stack_bounds;
  uint heap_pos;
} bnb_process_block_t;

typedef struct bnb_node {
  bnb_process_block_t proc_mem_info;
  struct bnb_node *next;
} bnb_node_t;

void bnb_node_clean(bnb_node_t *head);

bnb_node_t * bnb_find_node(int pid,bnb_node_t *head);

bnb_node_t * bnb_init_node(process_t process, uint fposition);

#endif