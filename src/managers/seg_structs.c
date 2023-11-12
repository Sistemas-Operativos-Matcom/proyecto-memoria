#include "seg_structs.h"
#include "free_list.h"

#include "stdio.h"

void seg_node_clean(seg_node_t *head)
{
    seg_node_t *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

seg_node_t * seg_find_node(int pid,seg_node_t *head)
{
    seg_node_t *temp = head;

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

seg_node_t * seg_init_node ( process_t process )
{
    seg_node_t *current = (seg_node_t *) malloc(sizeof(seg_node_t));
    current->proc_mem_info.pid = process.pid;
    current->proc_mem_info.cod_base = find_freelist_node(process.program->size);
    current->proc_mem_info.cod_bounds=process.program->size;
    m_set_owner( current->proc_mem_info.cod_base, current->proc_mem_info.cod_base + current->proc_mem_info.cod_bounds);
    current->proc_mem_info.stack_base = find_freelist_node(STACK_SEG_SIZE);
    current->proc_mem_info.stack_pos = STACK_SEG_SIZE;
    m_set_owner( current->proc_mem_info.stack_base, current->proc_mem_info.stack_base + STACK_SEG_SIZE);
    current->proc_mem_info.heap_base = find_freelist_node(HEAP_SEG_SIZE);
    current->proc_mem_info.heap_pos = 0;
    m_set_owner( current->proc_mem_info.heap_base, current->proc_mem_info.heap_base + HEAP_SEG_SIZE);

    return current;
}