#include <stdio.h>
#include <stdlib.h>
#include "FreeList.c"

#include "stdio.h"
#include "utils.h"

// El context es el contenedor de todos los datos necesarios 
// sobre un proceso que este guardado en la memoria. 
typedef struct Context
{
    int pid; 
    int size;
    addr_t base;
    addr_t bound;
    addr_t stack_pointer;
    addr_t heap_pointer;
    int stack_count; 
    int* stack; 
    Free_list_t heap_free_list;
}  Context_t; 

Context_t* BuildContext(int context_size, int adr,process_t *proc) 
{
    Context_t* con;
    con = malloc(sizeof(int)*3+sizeof(addr_t)*4);
    con->pid = proc->pid;
    con->size = proc->program->size;
    con->base = adr; 
    con->bound = adr + context_size;
    con->stack_pointer = adr + context_size;
    con->heap_pointer = con->size;
    con->stack = malloc(sizeof(int)*context_size);
    con->stack_count = 0;
    con->heap_free_list = Build_Free_List(context_size);
    return con;
} 
int context_push(Context_t* con, int val)
{
    if(con->stack_pointer-1<=con->heap_pointer)
      return -1;
    con->stack[con->stack_count] = val;
    con->stack_count++;
    con->stack_pointer--;
    return 1;
}
int context_pop(Context_t* con)
{
    int index = con->stack_count-1;
    int result = con->stack[index];
    con->stack_count--;
    con->stack_pointer++;
    return result;
}
int context_malloc(Context_t* con, int size)
{
    int result = allocate_space(&con->heap_free_list,size);
    con->heap_pointer = get_last_position(&con->heap_free_list);
    if(con->heap_pointer >= con->stack_pointer)
      return -1;
    return result;
}
int context_free(Context_t* con, int adr, int size)
{
    int result = free_space(&con->heap_free_list,adr,size);
    con->heap_pointer = get_last_position(&con->heap_free_list);
    return result; 
}
