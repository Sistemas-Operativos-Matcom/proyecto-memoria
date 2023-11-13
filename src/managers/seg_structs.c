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
