#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "process_pag.h"
#include "process_list.h"
#include <stddef.h>

// Inicializa la estructura
process_List_t *p_init()
{
    process_List_t *l = malloc(sizeof(process_List_t));
    l->len = 0;
    l->size = 10;
    l->data = calloc(l->size, sizeof(process_pag_t *));

    return l;
}

// Resetea la estructura ya instanciada
void p_reset(process_List_t *l)
{
    free(l->data);                                       // Free the memory allocated for the data array
    l->len = 0;                                          // Reset the length to 0
    l->size = 10;                                        // Reset the size to the default value
    l->data = malloc(l->size * sizeof(process_pag_t *)); // Allocate memory for the data array
}

// Devuelve el proceso de una posición
process_pag_t *p_get(process_List_t *l, int i)
{
    if (i < l->len)
        return l->data[i];
    else
    {
        printf("index out of range");
        exit(1);
    }
}

// Setea un valor en una posición
void p_set(process_List_t *l, int i, process_pag_t *c)
{
    if (i < l->len)
    {
        l->data[i] = c;
    }
}

// Aumenta el tamaño de la lista si es necesario
void p_increaseSize(process_List_t *l)
{
    l->size = l->size * 2;
    process_pag_t **newData = realloc(l->data, l->size * sizeof(process_pag_t *));
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
void p_push(process_List_t *l, process_pag_t *c)
{
    if (l->len == l->size)
        p_increaseSize(l);
    l->data[l->len] = c;
    l->len++;
}

process_pag_t *p_pop(process_List_t *l)
{
    process_pag_t *result = p_get(l, l->len - 1);
    l->len--;
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = realloc(l->data, l->size * sizeof(process_pag_t *));
    }
    return result;
}

// Función para eliminar el elemento de una posición específica de la lista
void p_deleteAt(process_List_t *l, int i)
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
        l->data = realloc(l->data, l->size * sizeof(process_pag_t *));
    }
}
