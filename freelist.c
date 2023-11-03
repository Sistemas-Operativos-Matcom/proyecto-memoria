// #include "aux.h"
#include <stdio.h>
#include <stdlib.h>


// Estructura para representar un nodo de la lista
typedef struct free_space {
    int base;
    int bound;
    struct free_space* next;
} free_space;

// Función para crear un nuevo nodo
free_space* createfree_space(int base, int bound) {
    free_space* newfree_space = (free_space*)malloc(sizeof(free_space));
    newfree_space->base = base;
    newfree_space->bound = bound;
    newfree_space->next = NULL;
    return newfree_space;
}

// Función para agrupar los espacios vacios adyacentes
void zip(free_space* temp) {
    while (temp->next != NULL)
    {
        if (temp->base + temp->bound == temp->next->base)
        {
            temp->bound = temp->bound + temp->next->bound;
            temp->next = temp->next->next;
        }

        if (temp->next == NULL)
        {
            break;
        }
        

        if (temp->base + temp->bound != temp->next->base)
        {
            temp = temp->next;   
        }
    }
}

// Función para insertar un elemento al final de la lista
void insert(free_space* head, int base, int bound) {
    free_space* newfree_space = createfree_space(base, bound);
    
    if (head->base == -2 && head->next == NULL) {
        head->next = newfree_space;
    }
    else
    {
        // Iterar por los espacios libres hasta encontrar el lugar en el que debería ir este espacio nuevo
        while (head->next != NULL && head->next->base < base)
        {
            head = head->next;
        }
        newfree_space->next = head->next;
        head->next = newfree_space;

        zip(head);
        
    }

    printf("Elemento insertado correctamente.");
}

// Reservar espacio en la FreeList
void delete(free_space* head, int bound) {
    if (head->base == -2 && head->next == NULL) {
        printf("La lista está vacía.\n");
        return;
    }

    // Buscar el lugar para ocupar
    while (head->bound < bound && head->next != NULL)
    {
        head = head->next;
    }

    if(head->base == -2)
    {
        printf("Imposible reservar, no hay espacio libre\n");
        return;
    }

    if(head->bound > bound)
    {
        head->bound = head->bound - bound;
        printf("Espacio reservado correctamente\n");
        return;
    }
    
}

// Función de prueba para imprimir los elementos de la lista
void printList(free_space* head) {
    if (head->base == -2 && head->next == NULL) {
        printf("La lista está vacía.\n");
        return;
    }
    
    printf("Elementos en la lista: ");
    while (head->next != NULL) {
        printf(": %d - ", head->next->base);
        printf("%d :", head->next->bound);
        head = head->next;
    }
    printf("\n");
}

int main(int argc, char const *argv[])
{
    free_space* freelist = createfree_space(-2, 0);

    insert(freelist, 5, 10);
    printList(freelist);
    insert(freelist, 25, 3);
    printList(freelist);
    insert(freelist, 15, 10);
    printList(freelist);
    delete(freelist, 10);
    printList(freelist);
    // printf("hello");
    return 0;
}

//         5-10          15-10         25-3
// ----- | ----- ----- | ----- ----- | ---

