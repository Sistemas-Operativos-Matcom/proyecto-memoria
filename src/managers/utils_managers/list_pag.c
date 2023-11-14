#include <stdio.h>
#include <stdlib.h>
#include "list_pag.h"
list *Init_list_of_pages()
{
    list *l = (list *)malloc(sizeof(list));
    l->length = 0;
    l->size = 8;
    l->data = (process_pag *)malloc(sizeof(process_pag) * l->size);
    return l;
}
int pag_Push(list *l, process_pag proc_to_insert)
{
    if (l->size == l->length + 1)
    {
        l->size = l->size * 2;
        l->data = (process_pag *)realloc(l->data, sizeof(process_pag) * l->size);
    }
    l->data[l->length] = proc_to_insert;
    l->length = l->length + 1;
    return 1;
}
int pag_Contains(list *l, int pid)
{
    for (size_t i = 0; i < l->length; i++)
    {
        process_pag process = l->data[i];
        if (process.pid == pid)
        {
            return i;
        }
    }
    return -1;
}
int pag_RemovePos(list *l, size_t pos)
{
    if (l->length <= pos)
        return -1;
    for (size_t i = pos; i < l->length; i++)
    {
        l->data[i] = l->data[i + 1];
    }
    l->length = l->length - 1;
    return 1;
}
int pag_Free(list *l)
{
    free(l->data);
    free(l);
    return 1;
}