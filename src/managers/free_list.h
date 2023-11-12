#include "../memory.h"
#include "../utils.h"

#include "stdio.h"
//estructura donde se guardan los bloques de memoria libres
typedef struct fl_mem_info {
  unsigned long addr;//direccion
  unsigned long len;//tamaño
} fl_mem_info_t;

//lista de los espacios de memoria libres
typedef struct fl_mem_info_node {
  fl_mem_info_t block_info;//información del bloque
  struct fl_mem_info_node *next;//próximo bloque
} fl_mem_info_node_t;



void fl_clear(); 
void fl_init(size_t size);
addr_t fl_alloc(size_t size);
void fl_free(size_t from_addr, size_t to_addr);
void fl_show();