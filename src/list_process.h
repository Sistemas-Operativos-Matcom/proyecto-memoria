#ifndef LIST_PROCESS_H
#define LIST_PROCESS_H

#include <stdio.h>
#include <malloc.h>
#include "utils.h"
#include "list_int.h"
#include "freelist.h"

typedef struct Process
{
  List_int *heap_pages;
  List_int *stack_pages;
  size_t SP;
  int process_pid;
  free_list *heap;
} Process;

typedef struct List_process
{
  Process *list_start;
  int length;
  int size;
} List_process;

List_process *new_list_process();

Process Get_index_process(int position, List_process *list);

void Set_index_process(int position, Process value, List_process *list);

void Insert_process(int position, Process value, List_process *list);

void Delete_process(int position, List_process *list);

void Push_process(Process value, List_process *list);

Process Pop_process(List_process *list);

void Free_List_process(List_process *list);

#endif