#ifndef FREE_LIST_H
#define FREE_LIST_H

#include "../memory.h"
#include "../utils.h"

#include "stdio.h"

typedef struct freelist {
  uint base;
  uint bounds;
} freelist_t;

typedef struct freelist_node {
  freelist_t free_block;
  struct freelist_node *next;
} freelist_node_t;

void freelist_init(size_t size);//inicia la estructura de la free list
uint find_freelist_node(size_t size);//encuentra un espacio libre de tamano size 
void add_freelist_node(uint base,uint bounds);//anade un espacio liberado
void update_freelist();//actualiza la free list compactando todos los bloques libres consecutivos 

#endif