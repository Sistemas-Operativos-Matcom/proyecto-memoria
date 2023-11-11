#include "stdio.h"
#include <stdlib.h>

#include "data_structures.h"

free_list *create_fl(unsigned long size)
{
    free_list_node *tr = (free_list_node *)malloc(sizeof(free_list_node));
    tr->start = 0;
    tr->size = size;
    tr->next = ((void *)0);
    free_list *node = (free_list *)malloc(sizeof(free_list));
    node->head = tr;
    node->size = size;
    return node;
}

void free_list_free(free_list *f)
{
    free_list_node *st = f->head;
    while (st != ((void *)0))
    {
        free_list_node *prev = st;
        st = st->next;
        free(prev);
    }
    free(f);
}

int can_insert(free_list *l, unsigned long size)
{
    free_list_node *st = l->head;
    while (st != ((void *)0))
    {
        if (st->size >= size)
        {
            return 1;
        }
        st = st->next;
    }
    return 0;
}

int is_recoverable(free_list *l, unsigned long start, unsigned long size)
{
    int doable = 1;
    free_list_node *st = l->head;
    while (st != ((void *)0))
    {
        if (start + size <= st->start || start >= st->start + st->size)
        {
            st = st->next;
        }
        else
        {
            doable = 0;
            break;
        }
    }
    return doable;
}

void recover_space(free_list *l, unsigned long start, unsigned long size)
{
    int done = 0;
    free_list_node *st = l->head;
    while (st != ((void *)0))
    {
        if (st->start == start + size)
        {
            st->start -= size;
            st->size += size;
            done = 1;
            break;
        }
        if (st->start + st->size == start)
        {
            st->size += size;
            done = 1;
            break;
        }
        st = st->next;
    }
    if (!done)
    {
        free_list_node *neww = (free_list_node *)malloc(sizeof(free_list_node));
        neww->start = start;
        neww->size = size;
        neww->next = l->head;
        l->head = neww;
    }
    return;
}

unsigned long request_space(free_list *l, unsigned long size)
{
    free_list_node *prev = ((void *)0);
    free_list_node *st = l->head;
    while (st != ((void *)0))
    {
        if (st->size >= size)
        {
            if (st->size > size)
            {
                st->start += size;
                st->size -= size;
                return st->start - size;
            }
            else
            {
                unsigned long to_return = st->start;
                if (prev != ((void *)0))
                {
                    prev->next = st->next;
                }
                else
                {
                    l->head = st->next;
                }
                free(st);
                return to_return;
            }
        }
        prev = st;
        st = st->next;
    }
    return -1;
}

int is_occupied(free_list *l, unsigned long pos)
{
    if (pos >= l->size)
    {
        return 0;
    }
    int found = 0;
    free_list_node *st = l->head;
    while (st != ((void *)0))
    {
        if (pos >= st->start && pos < st->start + st->size)
        {
            found = 1;
            break;
        }
        st = st->next;
    }
    return !found;
}

void print_free_list(free_list *A)
{
    free_list_node *st = A->head;
    while (st != NULL)
    {
        fprintf(stderr, "Start: %lx, Size: %lu \n", st->start, st->size);
        st = st->next;
    }
}


void print_pcb(pcb *A)
{
    fprintf(stderr, "PCB with PID: %d\n", A->pid);
    fprintf(stderr, "code_start: %lx code_end: %lx\n", A->code_start, A->code_end);
    fprintf(stderr, "heap_start: %lx heap_end: %lx\n", A->heap_start, A->heap_end);
    fprintf(stderr, "stack_end: %lx stack_start: %lx\n", A->stack_end, A->stack_start);
    fprintf(stderr, "stack_pointer: %lx\n", A->stack_pointer);
    fprintf(stderr, "FREE NODES COMING \n");
    print_free_list(A->fl_heap);
}