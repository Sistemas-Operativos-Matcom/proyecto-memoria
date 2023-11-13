#include <stdlib.h>
#include <stdio.h>
#include "free_list.h"


void Init_free_list(free_list_t *free_list, size_t size)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node->num_pages = size;
    node->first_page_frame = 0;
    node->previous = node->next = NULL;
    free_list->size = 1;
    free_list->top = free_list->bottom = node;
    free_list->max_page_frame = size;
}

void Reset_free_list(free_list_t* list, size_t size) {
    if (list->size != 0) {
        // Vaciar la lista de los nodos existentes
        node_t* node = list->top;
        while (node != NULL)
        {
            node_t* next = node->next;
            free(node);
            node = next;
        }
        list->size = 0;
    }

    Init_free_list(list, size);
}

void Connect_previous(node_t *node, node_t *previous, free_list_t *free_list)
{
    if (previous != NULL)
        previous->next = node;
    else
        free_list->top = node;
    node->previous = previous;
}
void Connect_next(node_t *node, node_t *next, free_list_t *free_list)
{
    if (next != NULL)
        next->previous = node;
    else
        free_list->bottom = node;
    node->next = next;
}

int Get_from_memory(free_list_t *free_list, size_t size, size_t *addr)
{
    if (free_list->size == 0)
        return 1;
    node_t *node = free_list->top;
    while (node != NULL)
    {
        if (node->num_pages >= size)
        {
            *addr = node->first_page_frame;
            size_t new_num_pages = node->num_pages - size;
            size_t new_first_page = node->first_page_frame + size;
            node_t *new_node_previous = node->previous;
            node_t *new_node_next = node->next;
            if (new_num_pages != 0)
            {
                node_t *new_node = (node_t *)malloc(sizeof(node_t));
                new_node->first_page_frame = new_first_page;
                new_node->num_pages = new_num_pages;
                Connect_previous(new_node, new_node_previous, free_list);
                Connect_next(new_node, new_node_next, free_list);
            }

            else
            {
                new_node_previous->next = new_node_next;
                new_node_next->previous = new_node_previous;
                free_list->size--;
            }

            free(node);
            if (free_list->size == 0)
                free_list->top = free_list->bottom = NULL;
            return 0;
        }

        node = node->next;
    }
    return 1;
}

int Free_memory(free_list_t *free_list, size_t size, size_t addr)
{
    if (free_list->size == 0)
    {
        node_t *node = (node_t *)malloc(sizeof(node_t));
        node->first_page_frame = addr;
        node->num_pages = size;
        Connect_previous(node, NULL, free_list);
        Connect_next(node, NULL, free_list);
        return 0;
    }

    else
    {
        node_t *node = free_list->top;
        while (node != NULL)
        {
            size_t prev_last_page_frame = node->first_page_frame + node->num_pages;
            size_t next_first_page_frame = node->next != NULL ? node->next->first_page_frame : free_list->max_page_frame;

            if ((prev_last_page_frame <= addr && addr + size <= next_first_page_frame) || addr + size <= node->first_page_frame)
            {
                node_t *new_node = (node_t *)malloc(sizeof(node_t));
                free_list->size++;
                new_node->first_page_frame = addr;
                new_node->num_pages = size;

                if (addr + size <= node->first_page_frame)
                {
                    Connect_previous(new_node, NULL, free_list);
                    if (addr + size == node->first_page_frame)
                    {
                        new_node->num_pages += (node->num_pages);
                        Connect_next(new_node, node->next, free_list);
                        free_list->size--;
                        free(node);
                    }
                    else
                    {
                        
                        Connect_next(new_node, node, free_list);
                    }
                }
                else
                {
                    node_t *new_node_next = node->next;
                    if (prev_last_page_frame == addr)
                    {
                        new_node->num_pages += node->previous->num_pages;
                        new_node->first_page_frame = node->first_page_frame;
                        //new_node->first_page = node->first_page;
                        Connect_previous(new_node, node->previous, free_list);
                        free_list->size--;
                        free(node);
                    }
                    else
                    {
                        Connect_previous(new_node, node, free_list);
                    }
                    if (next_first_page_frame == addr + size)
                    {
                        new_node->num_pages += new_node_next->num_pages;
                        Connect_next(new_node, new_node_next->next, free_list);
                        free_list->size--;
                        free(new_node_next);
                    }
                    else
                    {
                        Connect_next(new_node, new_node_next, free_list);
                    }
                }
                return 0;
            }

            node = node->next;
        }
        return 1;
    }
}



void Init_virtual_process(virtual_process_t* proc, int pid, size_t size) {
    proc->pid = pid;
    proc->pfn = (size_t*) malloc(size * sizeof(size_t));
    proc->page_valid = (int*) malloc(size * sizeof(int));
    proc->total_mem = size;
    proc->active = 1;
    proc->stack_point = 0xffffffffffffffff;

    for (size_t i = 0; i < size; i++) {
        proc->page_valid[i] = 0;
    }
}


size_t Find_pages(const virtual_process_t* proc, size_t size) 
{
    size_t count = 0;
    size_t first_page = 0;
    
    for (size_t i = 0; i < proc->total_mem; i++) {
        if (count == size)return first_page;
        if (!proc->page_valid[i])count++;
        else {
            count = 0;
            first_page = i + 1;
        }
    }

    return ~0;
}

size_t Get_Log2(size_t n) 
{
    size_t x = -1;
    while (n) 
    {
        x ++;
        n >>= 1;
    }
    
    return x;
}
