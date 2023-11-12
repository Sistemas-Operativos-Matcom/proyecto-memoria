#include "free_list.h"

#include "stdio.h"

freelist_node_t *freelist = NULL;

void freelist_init(size_t size)
{
    freelist_node_t *temp;
    while ( freelist != NULL)
    {
        temp = freelist;
        freelist = freelist->next;
        free(temp);
    }
    freelist = (freelist_node_t *) malloc(sizeof(freelist_node_t));
    freelist->free_block.base = 0;
    freelist->free_block.bounds = size;
    freelist->next = NULL;
}

uint find_freelist_node(size_t size)
{
    freelist_node_t *temp = freelist;
    uint result = NULL;

    while ( temp != NULL )
    {
        if( temp->free_block.bounds >= size )
        {
            result = temp->free_block.base;
            temp->free_block.base = temp->free_block.base + size;
            temp->free_block.bounds = temp ->free_block.bounds - size;
            update_freelist();
            return result;
        }
    }

    return result;
} 

void add_freelist_node(uint base,uint bounds)
{
    freelist_node_t *temp = freelist;
    freelist = (freelist_node_t *) malloc(sizeof(freelist_node_t));
    freelist->free_block.base = base;
    freelist->free_block.bounds = bounds;
    freelist->next = temp;
    update_freelist();
}

void update_freelist()
{
    freelist_node_t *temp;
    freelist_node_t *aux;
    uint base;
    uint bounds;
    
    for(temp = freelist; temp != NULL; temp=temp->next) 
    {
        for(aux = temp->next; aux != NULL; aux=aux->next) 
        {
            if (aux->free_block.base < temp->free_block.base) 
            {
                base = temp->free_block.base;
                bounds = temp->free_block.bounds;
                temp->free_block.base = aux->free_block.base;
                temp->free_block.bounds = aux->free_block.bounds;
                aux->free_block.base = base;
                aux->free_block.bounds = bounds;
            }
        }
    }

    temp = freelist;
    while( temp != NULL) 
    {
        if(temp->next != NULL)
        {
            if (temp->free_block.base + temp->free_block.bounds >= temp->next->free_block.base) 
            {
                aux = temp->next;
                temp->free_block.bounds += temp->next->free_block.bounds;
                temp->next = temp->next->next;

                free(aux);

            }
            else
            {
                temp=temp->next;
            }
        }
        else
        {
            temp=temp->next; 
        }
    }
} 