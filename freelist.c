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
    
    if (head == NULL) {
        head = newfree_space;
    }
    else
    {
        free_space* temp = head;
        // Iterar por los espacios libres hasta encontrar el lugar en el que debería ir este espacio nuevo
        while (temp->next != NULL && temp->next->base < base)
        {
            temp = temp->next;
        }
        newfree_space->next = temp->next;
        temp->next = newfree_space;

        zip(temp);
        // // Revisar si puedo fusionar el espacio que quiero añadir al anterior
        // if (temp->base + temp->bound == base)
        // {
        //     temp->bound = temp->bound + bound;
        //     printf("Elemento %d actualizado correctamente.\n", temp->base);
        //     return;
        // }

        // // Revisar si puedo fusionar el espacio que quiero añadir al siguiente
        // if (temp->next != NULL && base + bound == temp->next->base)
        // {
        //     temp->next->base = base;
        //     temp->next->bound = temp->next->bound + bound;
        //     printf("Elemento %d actualizado correctamente.\n", base);
        //     return;
        // }
        
        
        
    }

    // printf("Elemento insertado correctamente.");
}

// Reservar espacio en la FreeList
void delete(free_space* head, int bound) {
    if (head == NULL) {
        printf("La lista está vacía.\n");
        return;
    }

    free_space* current = head;

    // if (current->base == base) {
    //     head = current->next;
    //     free(current);
    //     printf("Elemento %d borrado correctamente.\n", base);
    //     return;
    // }

    // while (current != NULL && current->base != base) {
    //     previous = current;
    //     current = current->next;
    // }

    // if (current == NULL) {
    //     printf("Elemento %d no encontrado en la lista.\n", base);
    //     return;
    // }

    // previous->next = current->next;
    // free(current);
    // printf("Elemento %d borrado correctamente.\n", base);

    // Buscar el lugar para ocupar
    while (current->bound < bound && current->next != NULL)
    {
        current = current->next;
    }

    if(current == NULL)
    {
        printf("Imposible reservar, no hay espacio libre");
        return;
    }

    if(current->bound > bound)
    {
        current->bound = current->bound - bound;
        printf("Espacio reservado correctamente");
        return;
    }
    
}

// Función de prueba para imprimir los elementos de la lista
void printList(free_space* head) {
    if (head == NULL) {
        printf("La lista está vacía.\n");
        return;
    }
    free_space* temp = head;
    printf("Elementos en la lista: ");
    while (temp != NULL) {
        printf(": %d - ", temp->base);
        printf("%d :", temp->bound);
        temp = temp->next;
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
