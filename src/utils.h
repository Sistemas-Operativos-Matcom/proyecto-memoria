#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

#define max_mem 1000000  
#define max_pro 1000  
#define max_pag 100  
#define pag_size 1000
#define stack_size 1000  

// Esta estructura representa un puntero. No puedes cambiar el nombre ni
// eliminar la estructura. Puedes agregar campos nuevos si es necesario.
typedef struct ptr {
  addr_t addr;  // No eliminar este campo
  size_t size;
} ptr_t;

typedef struct program {
  char *name;
  size_t size;
} program_t;

typedef struct process {
  int pid;
  program_t *program;
  int base;
} process_t;

typedef struct node {
  int addr;
  int len;
  struct node *next;
} node_t;

node_t init_linked_list(int addr_,int len_);
void add_node(node_t *root,int addr_,int len_);
void upd_linked_list(node_t *root);
void delete_node(node_t *root,int addr_);

typedef struct Block{
    addr_t heap;
    addr_t stack;
    size_t tam;
    addr_t start;
    addr_t end;
    int usuario;
    int en_uso;
} Block_t;

struct page_table{
    int page_f[max_pag];
    int used[max_pag];
    int mk[max_pag][pag_size];
}table[max_pro];


program_t new_program(char *name, size_t size);
process_t new_process(int pid, program_t *program);

#endif
