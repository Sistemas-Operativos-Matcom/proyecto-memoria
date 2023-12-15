#include <stdio.h>
#include <malloc.h>
#include "list_process.h"

List_process *new_list_process()
{
    List_process *new = malloc(sizeof(List_process));
    new->list_start = malloc(32 * sizeof(int));
    new->length = 0;
    new->size = 32;

    return new;
}

void Check_size_process(List_process *list)
{
    if (list->length + 1 >= list->size)
    {
        list = realloc(list->list_start, list->size * 2);
    }
}

Process Get_index_process(int position, List_process *list)
{
     
    return list->list_start[position];
}

void Set_index_process(int position, Process value, List_process *list)
{
    if (position >= list->length || position < 0)
    {
        return;
    }
    list->list_start[position] = value;
}

void Insert_process(int position, Process value, List_process *list)
{
    if (position >= list->length || position < 0)
    {
        return;
    }

    Check_size_process(list);

    for (int i = list->length; i > position; i--)
    {
        list->list_start[i] = list->list_start[i - 1];
    }
    list->list_start[position] = value;
    list->length++;
}

void Delete_process(int position, List_process *list)
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

void Push_process(Process value, List_process *list)
{
    Check_size_process(list);
    list->list_start[list->length] = value;
    list->length++;
}

Process Pop_process(List_process *list)
{
    list->length--;
    return list->list_start[list->length + 1];
}

void Free_List_process(List_process *list)
{
    free(list->list_start);
    free(list);
}
