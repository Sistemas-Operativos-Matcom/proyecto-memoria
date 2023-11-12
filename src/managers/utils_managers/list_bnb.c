#include <stdio.h>
#include <stdlib.h>
#include "list_bnb.h"
list *Init_list()
{
    list *l = (list *)malloc(sizeof(list));
    l->length = 0;
    l->size = 8;
    l->data = (process_bnb *)malloc(sizeof(process_bnb) * l->size);
    return l;
}
int bnb_Free(list *l)
{
    free(l->data);
    free(l);
    return 1;
}
int bnb_Push(list *l, process_bnb proc_to_insert)
{
    if (l->size == l->length + 1)
    {
        l->size = l->size * 2;
        l->data = (process_bnb *)realloc(l->data, sizeof(process_bnb) * l->size);
    }
    l->data[l->length] = proc_to_insert;
    l->length = l->length + 1;
    return 1;
}
int bnb_RemovePos(list *l, size_t pos)
{
    // elimino el valor en la posicion pos y corro una pos a la izquierda el resto de los elementos
    if (l->length <= pos)
        return -1;
    for (size_t i = pos; i < l->length; i++)
    {
        l->data[i] = l->data[i + 1];
    }
    l->length = l->length - 1;
    return 1;
}
int bnb_Contains(list *l, int pid)
{
    for (size_t i = 0; i < l->length; i++)
    {
        process_bnb proces = l->data[i];
        if (proces.pid == pid)
        {
            return i;
        }
    }
    return -1;
}