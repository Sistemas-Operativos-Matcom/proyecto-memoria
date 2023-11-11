#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#include <stddef.h>

// Inicializa la estructura
sizeList_t *init()
{
    sizeList_t *l = (sizeList_t *)malloc(sizeof(sizeList_t));
    l->len = 0;
    l->size = 10;
    l->data = (size_t *)malloc(l->size * sizeof(size_t));
    return l;
}

// Resetea la estructura ya instanciada
void reset(sizeList_t *l)
{
    l->len = 0; // Reset the length to 0

    // Free every element of the data array
    for (size_t i = 0; i < l->size; i++)
    {
        free((void *)l->data[i]);
    }

    free(l->data);                                                  // Free the memory allocated for the data array
    l->size = (size_t)10;                                           // Reset the size to the default value
    l->data = (size_t *)realloc(l->data, l->size * sizeof(size_t)); // Allocate memory for the data array
}

// Devuelve el caracter de una posición
size_t get(sizeList_t *l, size_t i)
{
    if (i < l->len)
        return l->data[i];
    else
        exit(1);
}

// Setea un valor en una posición
void set(sizeList_t *l, size_t i, size_t c)
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
    size_t *newData = (size_t *)realloc(l->data, l->size * sizeof(size_t));
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
void push(sizeList_t *l, size_t c)
{
    if (l->len == l->size)
        increaseSize(l);
    l->data[l->len] = c;
    l->len++;
}

size_t pop(sizeList_t *l)
{
    size_t result = get(l, l->len - 1);
    l->len--;
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = (size_t *)realloc(l->data, l->size * sizeof(size_t *));
    }
    return result;
}

// Función para eliminar el elemento de una posición específica de la lista
void deleteAt(sizeList_t *l, size_t i)
{
    if (i >= l->len)
    { // Verificar si la posición es válida
        exit(1);
    }

    // Desplazar los elementos a la izquierda para sobrescribir el elemento en la posición i
    for (size_t j = i; j < l->len - 1; j++)
    {
        l->data[j] = l->data[j + 1];
    }

    l->len--; // Reducir el tamaño de la lista

    // Si el tamaño de la lista es menor que la mitad del tamaño del array, reducir el tamaño del array a la mitad
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = (size_t *)realloc(l->data, l->size * sizeof(size_t));
    }
}
