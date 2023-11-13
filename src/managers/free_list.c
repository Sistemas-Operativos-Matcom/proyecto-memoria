#include <stdlib.h>
#include <stdio.h>
#include "free_list.h"

void connect_nodes(node_t *node, node_t *next, node_t *prev, free_list_t *list)
{
    node->next = next;
    if (next != NULL)
        next->previous = node;
    else
        list->bottom = node; // Si su siguiente es nulo, entonces es la cola
    node->previous = prev;
    if (prev != NULL)
        prev->next = node;
    else
        list->top = node; // Si su anterior es nulo, entonces es la cabeza
}

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

int Free_memory(free_list_t* list,size_t size, size_t addr) {
    if (size > list->max_page_frame) return 1;

    // Si la lista esta vacia significa que se reservo toda la
    // memoria por lo que es valido liberarlo
    if (list->size == 0) {
        node_t* tmp = (node_t*) malloc(sizeof(node_t));

        tmp->next = tmp->previous = NULL;
        tmp->first_page_frame = addr;
        tmp->num_pages = size;

        return 0;
    }

    int status = 1;

    node_t* node = list->top;
    while (node != NULL) {
        size_t prv_idx = node->first_page_frame + node->num_pages;
        //size_t nxt_idx = (node->next != NULL) ? node->next->pos + node->next->size : list->max_pos;
        size_t nxt_idx = (node->next != NULL) ? node->next->first_page_frame: list->max_page_frame;

        // Buscar el espacio que deberia ser liberado
        if ((prv_idx <= addr && addr + size <= nxt_idx) || (addr + size  <= node->first_page_frame)) { //OJO
            // Crear un nuevo nodo para la memoria liberada
            node_t* tmp = (node_t*) malloc(sizeof(node_t));

            // Asignarle una posicion y un tamaño
            tmp->first_page_frame = addr;
            tmp->num_pages = size;

            node_t* prev = node->previous;
            node_t* next = node->next;

            //Actualizar el tamaño de la lista
            list->size++;

            // Caso corner: la memoria a liberar esta al inicio
            if (addr + size  <= node->first_page_frame) {
                list->top = tmp;

                // Si los nodos colisionan, se unen
                if (node->first_page_frame == addr + size ) {
                    tmp->next = node->next;
                    tmp->num_pages += node->num_pages;
                    free(node);
                }
                // Sino, solo se conectan
                else {
                    tmp->next = node;
                }

                status = 0;
                break;
            }

            // Si el nodo anterior colisiona con el actual, se fusionan
            // if (prev != NULL && (size_t) prev->pos + prev->size == tmp->pos) {
            //     fl_node_t* _prev = prev->prev;

            //     // Actualizar el nodo actual
            //     tmp->pos = prev->pos;
            //     tmp->size += prev->size;

            //     // Eliminar el nodo anterior ya que se fusiono con el actual
            //     free(prev);
            //     prev = _prev;

            //     //Actualizar el tamaño de la lista
            //     list->size--;
            // }

            // Si el nodo siguiente colisiona con el actial, se fusionan
            if (next != NULL && (size_t) tmp->first_page_frame + tmp->num_pages == next->first_page_frame) {
                node_t* _next = next->next;

                // Actualizar el nodo actual, como la pos del actual es menor que
                // la del siguiente, no es necesario actualizarlo
                tmp->num_pages += next->num_pages;

                // Eliminar el nodo siguiente
                free(next);
                next = _next;

                //Actualizar el tamaño de la lista
                list->size--;
            }

            // Conectar el nuevo nodo a sus vecinos
            connect_nodes(tmp, next, prev, list);

            // Terminar el ciclo y marcar el exito
            status = 0;
            break;
        }
        node = node->next;
    }

    // Si la lista se vacio, poner los punteros a null
    if (list->size == 0) {
        list->top = list->bottom = NULL;
    }

    return status;
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
