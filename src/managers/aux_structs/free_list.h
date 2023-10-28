#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdlib.h>

typedef struct fl_node
{
    size_t pos;
    size_t size;
    struct fl_node *prev;
    struct fl_node *next;
}fl_node_t;

typedef struct free_list
{
    fl_node_t* head;
    fl_node_t* tail;

    size_t size;
    size_t max_pos;
}free_list_t;

// Inicializa una lista a partir de un puntero
void fl_init_list(free_list_t* list, size_t total_mem);
// Vacia una lista ya creada (usar con cuidado!!!)
void fl_reset_list(free_list_t* list, size_t total_mem);
// Obtiene de la lista el primer segmento con al menos
// size bytes y pone su direccion en addr
int fl_get_memory(free_list_t* list,size_t size, size_t* addr);
// Se libera (de ser posible) un trozo de memoria de tamaño size
// que se encuentre en la posicion addr
int fl_free_memory(free_list_t* list,size_t size, size_t addr);

#endif