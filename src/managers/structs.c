#include <stdio.h>
#include <malloc.h>
#include "structs.h"

// Métodos de la Lista

List Init() // Inicializa la lista de base and bounds
{
    bandb *start = malloc(64 * sizeof(bandb));
    List new = {start, 0, 64};
    return new;
}

void Delete(int position, List *list) // Elimina el elemento en la posición dada
{
    for (int i = position; i < list->length; i++)
    {
        list->list_start[i] = list->list_start[i + 1];
    }
    list->length--;
}

void Remove(bandb value, List *list) // Elimina un elemento de la lista
{
    for (int i = 0; i < list->size; i++)
    {
        if (value.process.pid == list->list_start[i].process.pid)
        {
            Delete(i, list);
        }
    }
}

bandb *Find(process_t process, List *list) // Devuelve una referencia del objeto que se quiere buscar
{
    for (int i = 0; i < list->length; i++)
    {
        if (list->list_start[i].process.pid == process.pid)
            return &list->list_start[i];
    }
    return NULL;
}

void Check_Size(List *list) // Método Auxiliar de la lista para checkear el tamaño de la lista
{
    if (list->length + 1 >= list->size)
        list = realloc(list->list_start, list->size * 2);
}

void FreeList(List *list) // Libera el espacio de la lista en memoria
{
    free(list->list_start);
}

int Exist(process_t process, List *list) // verifica si un elemento existe en la lista
{
    for (int i = 0; i < list->length; i++)
    {
        if (list->list_start[i].process.pid == process.pid)
            return 1;
    }
    return 0;
}

void Push(bandb value, List *list)
{
    Check_Size(list);
    list->list_start[list->length] = value;
    list->length++;
}

// Métodos de la Linked List

LFList Init_LF(byte size)
{
    LFelement node = {0, size, NULL, NULL};
    LFList list = {&node};
    return list;
}

void Free_Space(byte address, byte size, LFList *list)
{
    if (list->first == NULL)
    {
        LFelement new = {address, size, NULL, NULL};
        list->first = &new;
        return;
    }

    LFelement *node = list->first;
    while (node->next != NULL || node->start < address)
    {
        node = node->next;
    }
    if (node->previous->start + node->previous->size == address)
    {
        node->previous->size += size;
        if (node->start == address + size)
        {
            node->previous->size += node->size;
            node->previous->next = node->next;
            free(node);
        }
    }
    else if (node->start == address + size)
    {
        node->start = address;
        node->size += size;
    }
    else
    {
        LFelement new = {address, size, node->previous, node};
        node->previous->next = &new;
        node->previous = &new;
    }
}

LFelement *Fill_Space(byte size, LFList *list)
{
    LFelement *node = list->first;
    while (node->size < size)
    {
        node = node->next;
    }

    if (node->size == size)
    {
        node->previous->next = node->next;
        node->next->previous = node->previous;
        return node->previous;
    }
    else
    {
        node->size -= size;
        return node;
    }
}
