#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <limits.h>
#include "memory.h"
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>
#include "utils.h"

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
} process_t;

//representa un espacio libre
typedef struct free_space {
  //direccion de ese espacio libre
  addr_t addr;
  //tamano del espacio libre
  int size;
} free_space_t;

//linked list de espacios libres ( free_list)
typedef struct node {
  free_space_t data;
  struct node *next;
  struct node *prev;
} node_t;

//representacion de un proceso en bnb
typedef struct item{
  addr_t base;
  int bound;
  process_t process;
  //free list del proceso
  node_t* free_list;
  addr_t stack_pointer;
}item_t;

//lista de procesos en bnb
typedef struct node_item {
  item_t item;
  struct node_item *next;
  struct node_item *prev;
} node_item_t;

//pagina
typedef struct  page{
  //id de la pagina
  int number;
  addr_t m_addr;
  int size;
  //free list de la pagina para manejar su espacio
  node_t* free_list;
}page_t;

//lista de paginas
typedef struct page_list{
  page_t page;
  struct page_list *prev;
  struct page_list *next;

}page_list_t;

//lista de enteros
typedef struct number_list {
  int data;
  struct number_list *next;
  struct number_list *prev;
} number_list_t;

//representacion de un proceso en paginacion
typedef struct page_item{
  int page_count;
  process_t process;
  addr_t stack_pointer;
  page_list_t* page_list_code;
  page_list_t* page_list_heap;
  page_list_t* page_list_stack;
  number_list_t* unpages;

}page_item_t;

//lista de procesos en paginacion
typedef struct node_page_item{
  page_item_t item;
  struct node_page_item *next;
  struct node_page_item *prev;
}node_page_item_t;





program_t new_program(char *name, size_t size);
process_t new_process(int pid, program_t *program);

#endif
