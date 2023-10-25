#include "stdio.h"
#include "utils.h"

typedef struct Context_info
{
    addr_t last_available_position;
    int* pid_context_arr;
    size_t pid_context_count;
} Context_info_t;

typedef struct Context
{
    process_t running_process;
    ptr_t pa;
    ptr_t heap_pa;
    ptr_t stack_pa;
} Context_t;


Context_info_t* BuildContextInfo()
{
    Context_info_t context ;
    context.last_available_position = 0;
// undone 
}


typedef struct MyList
{
    int* arr; 
    int count; 
    // it has to be big enough
} MyList_t; 


MyList_t BuildMyList()
{
    MyList_t list;
    // This size may brake...
    list.arr = (int*) malloc(sizeof(int)*10000000);
    list.count = 0;
    return list;
}

void AddToList(MyList_t* list, int x)
{
    list->arr[list->count] = x;
    list->count++;
}

void DeleteElementInList(MyList_t* list, int x)
{
    for (int i = 0; i < list->count; i++)
    {
        if(list->arr[i] == x)
        {
            for (int j = i; j+1 < list->count; j++)
            {
                list->arr[j] = list->arr[j+1];
            }
            list->count--;
            return;
        }
    }
}




// 18,446,744,073,709,551,616    2^64.

// typedef struct program {
//   char *name;
//   size_t size;
// } program_t;

// typedef struct process {
//   int pid;
//   program_t *program;
// } process_t;

// typedef struct ptr {
//   addr_t addr;  // No eliminar este campo
//   size_t size;
// } ptr_t;