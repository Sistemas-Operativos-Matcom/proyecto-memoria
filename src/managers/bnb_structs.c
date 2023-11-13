#include "bnb_structs.h"

#include "stdio.h"

void bnb_node_clean(bnb_node_t *head)
{
    bnb_node_t *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

bnb_node_t * bnb_find_node(int pid,bnb_node_t *head)
{
    bnb_node_t *temp = head;

    while (temp != NULL) 
    {
        if (temp->proc_mem_info.pid == pid) 
        {
            return temp;    
        }
        temp = temp->next;
    }

    return NULL;
}

bnb_node_t * bnb_init_node ( process_t process , uint fposition) 
{
    bnb_node_t *current = (bnb_node_t *) malloc(sizeof(bnb_node_t));
    current->proc_mem_info.pid = process.pid;
    current->proc_mem_info.base = fposition;
    current->proc_mem_info.bounds = BLOCK_SIZE;
    current->proc_mem_info.stack_pos = current->proc_mem_info.bounds;
    current->proc_mem_info.stack_bounds = STACK_BLOCK_SIZE;
    current->proc_mem_info.cod_bounds = process.program->size;
    current->proc_mem_info.heap_pos= process.program->size;

    return current;
}