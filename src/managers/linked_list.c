#include "linked_list.h"

// buscar un espacio de memoria con suficiente espacio,
// para ser usado por un proceso o ser reservado
linked_list_t* find_fit(linked_list_t* node, size_t size){
    if(node == NULL) return NULL;

    if(node->start.size  >= size) return node;
    else return find_fit(node->next, size);
}

//actualiza el size en un elemento de la linked list
//si es 0 lo elimina
//si el espacio crece lo suficiente hasta alcanzar al proimo bloque se fucionan
void update(linked_list_t* node, size_t size){
    if(node == NULL) return;

    if(size > 0){
        //actualizar el nodo es queivalente a eliminarlo y a añadir uno nuevo con los valores de puntero y size nuevos        
        node->start.addr += node->start.size - size; // ahora el puntero al espacio de memoria que sobro esta hacia adelante(size es size actual - size a ocupar por eso esta formula)
        node->start.size = size;
    }
    else if(size == 0){
        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
    }
}

//buscar el nodo con ptr start o sino el sucesor en la linked list
linked_list_t *find(linked_list_t *node, ptr_t start){
    if(node == NULL) return NULL;

    if(node->start.addr >= start.addr) return node;
    return find(node->next, start);
}

//garantizar siempre nuevo
linked_list_t *insert(linked_list_t *node, ptr_t start)
{
    if(node == NULL) return NULL;

    node = find(node, start);

    if(node->start.addr > start.addr){
        linked_list_t *new_node = (linked_list_t *)malloc(sizeof(linked_list_t));

        //insertar el nodo nuevo en la linked list en orden por el addr del puntero
        new_node->next = node;
        new_node->prev = node->prev;
        new_node->start = start;
        new_node->start.size = start.size;
        node->prev->next = new_node;
        node->prev = new_node;
        
        if((new_node->start.addr + new_node->start.size) >= node->start.addr){ //defragmentar segmentos consecutivos del heap
            new_node->start.size += node->start.size;
            new_node->next = node->next;
            node->next->prev = new_node;
            
            free(node);
        }

        if((new_node->prev->start.addr + new_node->prev->start.size) >= new_node->start.addr){ //defragmentar segmentos consecutivos del heap
            new_node->prev->start.size += new_node->start.size;
            new_node->prev->next = new_node->next;
            new_node->next->prev = new_node->prev;

            free(new_node);
        }

        return new_node;
    }

    return NULL;
}

int is_free(linked_list_t *node, addr_t addr){
    if(node == NULL) return 0;
    
    if(addr >= node->start.addr && addr < node->start.addr + node->start.size) return 1;
    
    if(is_free(node->next, addr)) return 1;

    return 0;
}

int is_valid_ptr(linked_list_t *node, ptr_t ptr){
    if(node == NULL) return 0;

    if(node->next == NULL) return 0;

    //verificar si cabe
    if(ptr.addr >= node->start.addr + node->start.size && ptr.addr < node->next->start.addr){
        if(ptr.size <= ((node->start.addr + node->start.size) + (node->next->start.addr - (node->start.addr + node->start.size)) - ptr.addr)) return 1;
        return 0;
    }

    if(is_valid_ptr(node->next, ptr)) return 1;

    return 0;
}

void free_entire_list(linked_list_t *node){
    if(node == NULL) return;

    free_entire_list(node->next);
    free(node);
}