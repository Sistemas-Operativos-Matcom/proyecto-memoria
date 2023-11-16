#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <malloc.h>
#include "utils.h"

typedef struct List
{
    int *list_start;
    int length;
    int size;
} List;

List *new_list();

int GetList(int position, List *list);

void SetList(int position, int value, List *list);

void Insert(int position, int value, List *list);

void Delete(int position, List *list);

void Push(int value, List *list);

int Pop(List *list);

void FreeList(List *list);

#endif