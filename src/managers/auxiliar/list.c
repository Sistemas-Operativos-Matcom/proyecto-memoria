#include <stdio.h>
#include <stdlib.h>
#include "list.h"
IntList *Init_l()
{
    IntList *l = (IntList *)malloc(sizeof(IntList));
    l->len = 0;
    l->size = 10; // cantidad mayor que 0
    l->data = (process_bb *)malloc(l->size * sizeof(process_bb));
    return l;
}
int Free_l(IntList *l)
{
    free(l->data);
    free(l);
    return 1;
}
int Push_l(IntList *l, process_bb v)
{
    // inserto al final de la lista
    if (l->size == l->len + 1)
    {
        l->size = l->size * 2;
        l->data = (process_bb *)realloc(l->data, l->size * sizeof(process_bb));
    }
    l->data[l->len] = v;
    l->len = l->len + 1;
    return 1;
}
int RemovePos_l(IntList *l, size_t p)
{
    // elimino el valor en la posicion p y corro una pos a la izquierda el resto de los elementos
    if (l->len <= p)
        return -1;
    for (size_t i = p; i < l->len; i++)
    {
        l->data[i] = l->data[i + 1];
    }
    l->len = l->len - 1;
    return 1;
}
/* void Print(IntList *l)
{
    printf("\n [");
    for (int i = 0; i < l->len - 1; i++)
    {
        printf("%d,", l->data[i]);
    }
    printf("%d]\n", l->data[l->len - 1]);
} */
int Contains_l(IntList *l, int pid)
{
    for (size_t i = 0; i < l->len; i++)
    {
        process_bb proces = l->data[i];
        if (proces.pid == pid)
        {
            return i;
        }
    }
    return -1;
}