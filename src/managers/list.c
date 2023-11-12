#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"
#include <stdint.h>

// Create a new empty list and return a pointer to it
sizeList_t *init()
{
    sizeList_t *list = malloc(sizeof(sizeList_t));
    if (list == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    list->len = 0;   // Initialize the size to zero
    list->size = 10; // Initialize the size to some initial value
    // Allocate memory for the array using calloc

    list->data = calloc(list->size, sizeof(element_type));

    if (list->data == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    return list;
}

// Resetea la estructura ya instanciada
void reset(sizeList_t *l)
{
    l->len = 0; // Reset the length to 0

    free(l->data);                                   // Free the memory allocated for the data array
    l->size = 10;                                    // Reset the size to the default value
    l->data = calloc(l->size, sizeof(element_type)); // Allocate memory for the data array
}

// Devuelve el caracter de una posición
element_type get(sizeList_t *l, int i)
{
    if (i < l->len)
        return l->data[i];
    else
    {
        printf("error en m_store");
        exit(1);
    }
}

// Setea un valor en una posición
void set(sizeList_t *l, int i, element_type c)
{
    if (i < l->len)
    {
        l->data[i] = c;
    }
    else
        exit(1);
}

// Aumenta el tamaño de la lista si es necesario
void increaseSize(sizeList_t *l)
{
    l->size = l->size * 2;
    element_type *newData = realloc(l->data, l->size * sizeof(element_type));
    if (newData != NULL)
    {
        l->data = newData;
    }
    else
    {
        printf("Error allocating memory in");
        exit(1);
    }
}

// Inserta un elemento al final de la lista
void push(sizeList_t *l, element_type c)
{
    if (l->len == l->size)
    {
        increaseSize(l);
    }

    l->data[l->len] = c;
    l->len++;
}

element_type pop(sizeList_t *l)
{
    size_t result = get(l, l->len - 1);
    l->len--;
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = realloc(l->data, l->size * sizeof(element_type));
    }
    return result;
}

// Función para eliminar el elemento de una posición específica de la lista
void deleteAt(sizeList_t *l, int i)
{
    if (i >= l->len)
    { // Verificar si la posición es válida
        exit(1);
    }

    // Desplazar los elementos a la izquierda para sobrescribir el elemento en la posición i
    for (int j = i; j < l->len - 1; j++)
    {
        l->data[j] = l->data[j + 1];
    }

    l->len--; // Reducir el tamaño de la lista

    // Si el tamaño de la lista es menor que la mitad del tamaño del array, reducir el tamaño del array a la mitad
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = realloc(l->data, l->size * sizeof(element_type));
    }
}
