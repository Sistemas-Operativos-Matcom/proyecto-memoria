#include <stdio.h>
#include <malloc.h>
#include "list_int.h"

List_int *new_list_int()
{
    List_int *new = malloc(sizeof(List_int));
    new->list_start = malloc(32 * sizeof(int));
    new->length = 0;
    new->size = 32;

    return new;
}

void Check_Size(List_int *list)
{
    if (list->length + 1 >= list->size)
    {
        list = realloc(list->list_start, list->size * 2);
    }
}

int GetList(int position, List_int *list)
{
    if (position >= list->length || position < 0)
    {
        return 0;
    }
    return list->list_start[position];
}

void SetList(int position, int value, List_int *list)
{
    if (position >= list->length || position < 0)
    {
        return;
    }
    list->list_start[position] = value;
}

void Insert(int position, int value, List_int *list)
{
    if (position >= list->length || position < 0)
    {
        return;
    }

    Check_Size(list);

    for (int i = list->length; i > position; i--)
    {
        list->list_start[i] = list->list_start[i - 1];
    }
    list->list_start[position] = value;
    list->length++;
}

void Delete(int position, List_int *list)
{
    if (position >= list->length || position < 0)
    {
        return;
    }
    for (int i = position; i < list->length; i++)
    {
        list->list_start[i] = list->list_start[i + 1];
    }
    list->length--;
}

void Push(int value, List_int *list)
{
    Check_Size(list);
    list->list_start[list->length] = value;
    list->length++;
}

int Pop(List_int *list)
{
    list->length--;
    return list->list_start[list->length + 1];
}

void Free_List(List_int *list)
{
    free(list->list_start);
    free(list);
}
