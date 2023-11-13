#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <limits.h>
#include "memory.h"
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>

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

 //lista de enteros
 typedef struct int_l{
  int data; //numero
  struct int_l *next_int; //next
  struct int_l *prev_int; //previous
 } int_l_t;

//hacemos una free list para saber en todo momento todos los espacios libres que tenemos
//definimos las propiedades de un espacio libre
typedef struct free_space {
  addr_t addr; //direccion
  int size; //tamano
} free_space_t;

//declaramos las propiedades de cada nodo de la free list
typedef struct free_list {
  free_space_t data; //esapcio libre
  struct free_list *next; //next
  struct free_list *prev; //previous
} free_list_t;

//hacemos una lista de procesos
//definimos las propiedades de cada proceso
typedef struct procs_info {
  process_t data; //proceso
  struct free_list *head; //free list 
  addr_t base; //base
  int bounds; //bound
  addr_t stack_pointer; //stack pointer
} procs_info_t;

//definimos las propiedades de cada nodo
typedef struct linked_node{
  procs_info_t data; //proceso
  struct linked_node *next_node; //next
  struct linked_node *prev_node; //prevoius
} linked_node_t;

//definimos una pagina
typedef struct page{
  int number; //numero 
  int size; //tamano
  struct free_list *free_head;  //free list
  addr_t real_addr; //direccion en memoria real
} page_t;

//definimos una lista de paginas
typedef struct page_list{
  struct page_list *next_page; //next
  struct page_list *prev_page; //previous
  page_t page; //pagina
} page_list_t;

//definimos el proceso
 typedef struct pag_procs{
  process_t procs; //proceso
  addr_t stack_pointer; //stack pointer
  struct page_list *pag_heap; //lista de paginas del heap
  struct page_list *pag_stack;//lista de paginas del stack
  struct page_list *pag_cod;//lista de paginas del codigo
  int pag_max; //cantidad de paginas que ha ocupado
  struct int_l *procs_int_list; //lista de enteros de paginas que estan libres
 } pag_procs_t;

//lista de procesos
 typedef struct pag_procs_list{
  pag_procs_t procs; //proceso
  struct pag_procs_list *next_procs; //next
  struct pag_procs_list *prev_procs; //previous
 } pag_procs_list_t;
program_t new_program(char *name, size_t size);
process_t new_process(int pid, program_t *program);

#endif
