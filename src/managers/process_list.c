#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "process_pag.h"
#include "process_list.h"

// Inicializa la estructura
process_List_t *p_init()
{
    process_List_t *l = (process_List_t *)malloc(sizeof(process_List_t));
    l->len = 0;
    l->size = 10;
    l->data = (process_pag_t **)malloc(l->size * sizeof(process_pag_t *));
    return l;
}

// Resetea la estructura ya instanciada
void p_reset(process_List_t *l)
{
    for (int i = 0; i < l->len; i++)
    {
        free(l->data[i]); // Free each element pointed by data
    }
    free(l->data);                                                         // Free the memory allocated for the data array
    l->len = 0;                                                            // Reset the length to 0
    l->size = 10;                                                          // Reset the size to the default value
    l->data = (process_pag_t **)malloc(l->size * sizeof(process_pag_t *)); // Allocate memory for the data array
}

// Devuelve el proceso de una posición
process_pag_t *p_get(process_List_t *l, int i)
{
    if (i >= 0 && i < l->len)
        return l->data[i];
    else
    {
        printf("index out of range");
        exit(1);
    }
}

// Setea un valor en una posición
int p_set(process_List_t *l, int i, process_pag_t *c)
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
int p_validIndex(process_List_t *l, int i)
{
    if (i < 0 || i > l->len) // check i is valid
        return -1;
    return 0;
}

// Aumenta el tamaño de la lista si es necesario
void p_increaseSize(process_List_t *l)
{
    l->size = l->size * 2;
    process_pag_t **newData = (process_pag_t **)realloc(l->data, l->size * sizeof(process_pag_t *));
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
int p_insert(process_List_t *l, int i, process_pag_t *c)
{
    // index was invalid
    if (validIndex(l, i) == -1)
        return -1;

    if (l->len == l->size)
        p_increaseSize(l);

    for (int j = l->len; j > i; j--)
        l->data[j] = l->data[j - 1];

    l->data[i] = c;
    l->len++;

    return 0;
}

// Inserta un elemento al final de la lista
void p_push(process_List_t *l, process_pag_t *c)
{
    if (l->len == l->size)
        p_increaseSize(l);
    l->data[length(l)] = c;
    l->len++;
}

process_pag_t *p_pop(process_List_t *l)
{
    process_pag_t *result = get(l, l->len - 1);
    l->len--;
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = (process_pag_t **)realloc(l->data, l->size * sizeof(process_pag_t *));
    }
}

// Función para eliminar el elemento de una posición específica de la lista
int p_deleteAt(process_List_t *l, int i)
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
        l->data = (process_pag_t **)realloc(l->data, l->size * sizeof(process_pag_t *));
    }

    return 0;
}
