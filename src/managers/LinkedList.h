#include "../utils.h"

typedef struct LinkedList
{
    ptr_t start;
    struct LinkedList *next;
    struct LinkedList *prev;
} LinkedList_t;

// buscar un espacio de memoria suficiente
LinkedList_t* FindSpace(LinkedList_t* node, size_t size);

//actualiza el size en un elemento de la linked list
void Update(LinkedList_t* node, size_t size);

//buscar el nodo con ptr start o sino el sucesor
LinkedList_t *Find(LinkedList_t *node, ptr_t start);

LinkedList_t *Insert(LinkedList_t *node, ptr_t start);

//Revisar si una direccion de memoria esta desocupada
int IsFree(LinkedList_t *node, addr_t addr);

//Validar un puntero
int ValidAdddress(LinkedList_t *node, ptr_t ptr);
//Liberar toda la lista
void FreeList(LinkedList_t *node);