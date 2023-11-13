#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdlib.h>

typedef struct node
{
    struct node *next;
    struct node *previous;
    size_t first_page_frame;
    size_t num_pages;
}node_t;

typedef struct free_list
{
    node_t *top;
    node_t *bottom;
    size_t size;
    size_t max_page_frame;
}free_list_t;

typedef struct virtual_process
{   
    int pid;
    int active;
    size_t total_mem;
    size_t stack_point;
    size_t* pfn;
    int* page_valid;
}virtual_process_t;

//Inicializa la free list
void Init_free_list(free_list_t *free_list, size_t size);
void Reset_free_list(free_list_t* list, size_t size);
//Conecta un nodo con su anterior y actualiza el tope de la pila en caso de ser necesario
void Connect_previous(node_t *node, node_t *previous, free_list_t *free_list);
//Conecta un nodo con su siguiente y actualiza el fondo de la pila en caso de ser necesario
void Connect_next(node_t *node, node_t *next, free_list_t *free_list);
//Obtiene de la lista el primer nodo con al menos size páginas libres y devuelve la primera página en addr
int Get_from_memory(free_list_t *free_list, size_t size, size_t *addr);
// Libera, de ser posible, un pedazo de memoria de tamaño size que se encuentre en la posicion addr
int Free_memory(free_list_t *free_list, size_t size, size_t addr);
// void connect_nodes(node_t *node, node_t *next, node_t *prev, free_list_t *list);


// Inicializa un virtual_process con datos nuevos
void Init_virtual_process(virtual_process_t* proc, int pid, size_t size);
// Encuentra el primer espacio con size paginas contiguas vacias
size_t Find_pages(const virtual_process_t* proc, size_t size);
// Calcula el logaritmo en base 2 para enteros
size_t Get_Log2(size_t n);
#endif