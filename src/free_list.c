#include <stdio.h>
#include <stdlib.h>

#include "free_list.h"

void insert(FreeList* freeList, int value, int size) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->value = value;
    newNode->size = size;
    newNode->next = NULL;

    if (freeList->head == NULL) {
        freeList->head = newNode;
    } else {
        struct Node* temp = freeList->head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

void print(FreeList* freeList) {
    struct Node* temp = freeList->head;
    while (temp != NULL) {
        fprintf(stderr,"Bloque de memoria disponible de tamaño %d\n", temp->size);
        temp = temp->next;
    }
}

struct Node* search(FreeList* freeList, int size) {
    struct Node* temp = freeList->head;
    while (temp != NULL) {
        if (temp->size >= size) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

struct Node* searchL(FreeList* freeList) {
    struct Node* temp = freeList->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    return temp;
}

void delete(FreeList* freeList, struct Node* node) {
    if (node == freeList->head) {
        freeList->head = freeList->head->next;
    } else {
        struct Node* temp = freeList->head;
        while (temp->next != node) {
            temp = temp->next;
        }
        temp->next = node->next;
    }
}

void deleteLast(FreeList* freeList) {
    
    struct Node* temp = freeList->head;
    struct Node* temp2 = temp->next;

    if(temp2 == NULL)
    {
        temp->next = NULL;
        return;
    }

    while (temp2->next != NULL) {
        temp = temp2->next;
        temp2 = temp2->next;
    }

    temp->next = NULL;

}

// int main() {
//     FreeList freeList;
//     memset(&freeList, 0, sizeof(FreeList));

//     // Insertar bloques de memoria disponibles
//     insert(&freeList, 100);
//     insert(&freeList, 200);
//     insert(&freeList, 300);

//     // Imprimir bloques de memoria disponibles
//     printf("Bloques de memoria disponibles:\n");
//     print(&freeList);
//     printf("\n");

//     // Buscar bloque de memoria disponible de tamaño 150
//     struct Node* bloque = search(&freeList, 150);
//     if (bloque != NULL) {
//         printf("Bloque de memoria disponible de tamaño %d encontrado.\n", bloque->size);
//     } else {
//         printf("No se encontró un bloque de memoria disponible del tamaño deseado.\n");
//     }
//     printf("\n");

//     // Eliminar bloque de memoria disponible
//     delete(&freeList, bloque);

//     // Imprimir bloques de memoria disponibles después de eliminar un bloque
//     printf("Bloques de memoria disponibles después de eliminar un bloque:\n");
//     print(&freeList);

//     return 0;
// }