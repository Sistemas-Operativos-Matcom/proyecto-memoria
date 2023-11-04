#include <stdio.h>
#include <stdlib.h>
#include "list.h"

// Devuelve la cantidad de elementos de la lista
int length(sizeList_t *list)
{
    return list->len;
}

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
    for (int i = 0; i < l->size; i++)
    {
        free(l->data[i]);
    }

    free(l->data);                                                  // Free the memory allocated for the data array
    l->size = 10;                                                   // Reset the size to the default value
    l->data = (size_t *)realloc(l->data, l->size * sizeof(size_t)); // Allocate memory for the data array
}

// Devuelve el caracter de una posición
size_t get(sizeList_t *l, int i)
{
    if (i >= 0 && i < l->len)
        return l->data[i];
    else
        return -1;
}

// Setea un valor en una posición
int set(sizeList_t *l, int i, size_t c)
{
    if (i >= 0 && i < l->len)
    {
        l->data[i] = c;
        return 0;
    }
    else
        return -1;
}

// Verifica que el índice se encuentre dentro del tamaño de la lista
int validIndex(sizeList_t *l, int i)
{
    if (i < 0 || i > l->len) // check i is valid
        return -1;
    return 0;
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

// Inserta un elemento en una posición definida
int insert(sizeList_t *l, int i, size_t c)
{
    // index was invalid
    if (validIndex(l, i) == -1)
        return -1;

    if (l->len == l->size)
        increaseSize(l);

    for (int j = l->len; j > i; j--)
        l->data[j] = l->data[j - 1];

    l->data[i] = c;
    l->len++;

    return 0;
}

// Inserta un elemento al final de la lista
void push(sizeList_t *l, size_t c)
{
    if (l->len == l->size)
        increaseSize(l);
    l->data[length(l)] = c;
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
}
// Función para imprimir todos los elementos de la lista
void printAll(sizeList_t *l)
{
    printf("La lista tiene %d elementos:\n", l->len);
    for (int i = 0; i < l->len; i++)
    {
        printf("%c ", l->data[i]);
    }
    printf("\n");
}

// Función para imprimir el elemento de una posición específica de la lista
void printAt(sizeList_t *l, int i)
{
    if (i >= 0 && i < l->len)
    {
        printf("El elemento en la posición %d es: %c\n", i, l->data[i]);
    }
    else
    {
        printf("Posición inválida\n");
    }
}

// Función para eliminar el elemento de una posición específica de la lista
int deleteAt(sizeList_t *l, int i)
{
    if (i < 0 || i >= l->len)
    { // Verificar si la posición es válida
        return -1;
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
        l->data = (size_t *)realloc(l->data, l->size * sizeof(size_t));
    }

    return 0;
}
