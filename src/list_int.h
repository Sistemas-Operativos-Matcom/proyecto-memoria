#ifndef LIST_INT_H
#define LIST_INT_H

#include <stdio.h>
#include <malloc.h>
#include "utils.h"

typedef struct List_int
{
    int *list_start;
    int length;
    int size;
} List_int;

List_int *new_list_int();

int GetList(int position, List_int *list);

void SetList(int position, int value, List_int *list);

void Insert(int position, int value, List_int *list);

void Delete(int position, List_int *list);

void Push(int value, List_int *list);

int Pop(List_int *list);

void Free_List(List_int *list);

#endif