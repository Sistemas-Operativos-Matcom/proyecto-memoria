#include <stdlib.h>
#include <stdio.h>
#include "free_list.h"

#define FAIL 1
#define SUCCESS 0


static void connect_nodes(fl_node_t* node, fl_node_t* next, fl_node_t* prev, free_list_t* list) {
    node->next = next;
    if (next != NULL)next->prev = node;
    else list->tail = node; // Si su siguiente es nulo, entonces es la cola
    node->prev = prev;
    if (prev != NULL)prev->next = node;
    else list->head = node; // Si su anterior es nulo, entonces es la cabeza
}


void fl_init_list(free_list_t* list, size_t total_mem) { 
    list->size = 1;
    list->head = list->tail = (fl_node_t*) malloc(sizeof(fl_node_t));

    // Asignar valores iniciales a la lista
    list->head->next = list->head->prev = NULL;
    list->head->pos = 0;
    list->head->size = total_mem;
    list->max_pos = total_mem;
}

void fl_reset_list(free_list_t* list, size_t total_mem) {
    if (list->size != 0) {
        // Vaciar la lista de los nodos existentes
        fl_node_t* node = list->head;
        while (node != NULL)
        {
            fl_node_t* _node = node->next;
            free(node);
            node = _node;
        }
        list->size = 0;
    }

    fl_init_list(list, total_mem);
}

int fl_get_memory(free_list_t* list,size_t size, size_t* addr) {
    if (list->size == 0)return FAIL;

    int status = FAIL;

    // Iterar por todos los nodos de la lista
    fl_node_t* node = list->head;
    while (node != NULL) {
        // Verificar si hay espacio en este nodo 
        if (node->size >= size) {
            *addr = node->pos;

            fl_node_t* prev = node->prev;
            fl_node_t* next = node->next;

            size_t new_size = node->size - size;
            int new_pos = node->pos + size;
            // Dividir el nodo si no desaparece por completo
            if (new_size != 0) {
                fl_node_t* tmp = (fl_node_t*) malloc(sizeof(fl_node_t));
                
                // Conectar el nuevo nodo a sus vecinos
                connect_nodes(tmp, next, prev, list);

                tmp->pos = new_pos;
                tmp->size = new_size;

                //Actualizar el tamaño de la lista
                list->size++;
            }
            // Si el nodo desaparece, conectar los que lo rodean
            else {
                // Estas condiciones previenen error en caso de que se reserve toda la memoria
                if (prev != NULL)prev->next = next;
                if (next != NULL)next->prev = prev;

                //Actualizar el tamaño de la lista
                list->size--;

                // Actualizar la cabeza de la lista
                list->head = node->next;
            }
            
            // Eliminar el nodo viejo
            free(node);

            // Terminar el ciclo y marcar el exito
            status = SUCCESS;
            break;
        }
        node = node->next;
    }

    // Si la lista se vacio, poner los punteros a null
    if (list->size == 0) {
        list->head = list->tail = NULL;
    }

    return status;
}

int fl_free_memory(free_list_t* list,size_t size, size_t addr) {
    if (size > list->max_pos) return FAIL;

    // Si la lista esta vacia significa que se reservo toda la
    // memoria por lo que es valido liberarlo
    if (list->size == 0) {
        fl_node_t* tmp = (fl_node_t*) malloc(sizeof(fl_node_t));

        tmp->next = tmp->prev = NULL;
        tmp->pos = addr;
        tmp->size = size;
        
        return SUCCESS;
    }

    int status = FAIL;

    fl_node_t* node = list->head;
    while (node != NULL) {
        size_t prv_idx = node->pos + node->size;
        size_t nxt_idx = (node->next != NULL) ? node->next->pos + node->next->size : list->max_pos;
        
        // Buscar el espacio que deberia ser liberado
        if ((prv_idx <= addr && addr + size <= nxt_idx) || (addr + size - 1 <= node->pos)) {
            // Crear un nuevo nodo para la memoria liberada
            fl_node_t* tmp = (fl_node_t*) malloc(sizeof(fl_node_t));

            // Asignarle una posicion y un tamaño
            tmp->pos = addr;
            tmp->size = size;

            fl_node_t* prev = node->prev;
            fl_node_t* next = node->next;

            //Actualizar el tamaño de la lista
            list->size++;

            // Caso corner: la memoria a liberar esta al inicio
            if (addr + size - 1 <= node->pos) {
                list->head = tmp;
                
                // Si los nodos colisionan, se unen
                if (node->pos == addr + size - 1) {
                    tmp->next = node->next;
                    tmp->size += node->size;
                    free(node);
                }
                // Sino, solo se conectan
                else {
                    tmp->next = node;
                }

                status = SUCCESS;
                break;
            }

            // Si el nodo anterior colisiona con el actual, se fusionan
            if (prev != NULL && (size_t) prev->pos + prev->size == tmp->pos) {
                fl_node_t* _prev = prev->prev;
                
                // Actualizar el nodo actual
                tmp->pos = prev->pos;
                tmp->size += prev->size;

                // Eliminar el nodo anterior ya que se fusiono con el actual
                free(prev);
                prev = _prev;

                //Actualizar el tamaño de la lista
                list->size--;
            }

            // Si el nodo siguiente colisiona con el actial, se fusionan
            if (next != NULL && (size_t) tmp->pos + tmp->size == next->pos) {
                fl_node_t* _next = next->next;

                // Actualizar el nodo actual, como la pos del actual es menor que
                // la del siguiente, no es necesario actualizarlo
                tmp->size += next->size;

                // Eliminar el nodo siguiente
                free(next);
                next = _next;

                //Actualizar el tamaño de la lista
                list->size--;
            }

            // Conectar el nuevo nodo a sus vecinos
            connect_nodes(tmp, next, prev, list);
            
            // Terminar el ciclo y marcar el exito
            status = SUCCESS;
            break;
        }
        node = node->next;
    }

    // Si la lista se vacio, poner los punteros a null
    if (list->size == 0) {
        list->head = list->tail = NULL;
    }

    return status;
}
