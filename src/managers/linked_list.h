#include "../utils.h"

typedef struct linked_list
{
    ptr_t start;
    struct linked_list *next;
    struct linked_list *prev;
} linked_list_t;

// buscar un espacio de memoria con suficiente espacio,
// para ser usado por un proceso o ser reservado
linked_list_t* find_fit(linked_list_t* node, size_t size);

//actualiza el size en un elemento de la linked list
//si es 0 lo elimina
//si el espacio crece lo suficiente hasta alcanzar al proimo bloque se fucionan
void update(linked_list_t* node, size_t size);

//buscar el nodo con ptr start o sino el sucesor en la linked list
linked_list_t *find(linked_list_t *node, ptr_t start);

linked_list_t *insert(linked_list_t *node, ptr_t start);

//busca si una direccion de memoria esta libre
int is_free(linked_list_t *node, addr_t addr);

//busca si un puntero esta en uso compurba el address y el size
int is_valid_ptr(linked_list_t *node, ptr_t ptr);

void free_entire_list(linked_list_t *node);