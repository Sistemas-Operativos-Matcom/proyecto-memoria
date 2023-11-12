#include "LinkedList.h"

// buscar un espacio de memoria suficiente
LinkedList_t* FindSpace(LinkedList_t* node, size_t size){
    if(node == NULL) return NULL;

    if(node->start.size  >= size) return node;
    else return FindSpace(node->next, size);
}

//actualiza el size en un elemento de la linked list
void Update(LinkedList_t* node, size_t size){
    if(node == NULL) return;

    if(size > 0){
        node->start.addr += node->start.size - size; 
        node->start.size = size;
    }
    else if(size == 0){
        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
    }
}

//buscar el nodo con ptr start o sino el sucesor
LinkedList_t *Find(LinkedList_t *node, ptr_t start){
    if(node == NULL) return NULL;

    if(node->start.addr >= start.addr) return node;
    return Find(node->next, start);
}

//Insertar nodo
LinkedList_t *Insert(LinkedList_t *node, ptr_t start)
{
    if(node == NULL) return NULL;

    node = Find(node, start);

    if(node->start.addr > start.addr){
        LinkedList_t *new_node = (LinkedList_t *)malloc(sizeof(LinkedList_t));

        //Insertar el nodo nuevo en la linked list en orden por el addr del puntero
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
//Revisar si una direccion de memoria esta desocupada
int IsFree(LinkedList_t *node, addr_t addr){
    if(node == NULL) return 0;
    
    if(addr >= node->start.addr && addr < node->start.addr + node->start.size) return 1;
    
    if(IsFree(node->next, addr)) return 1;

    return 0;
}
//Validar un puntero
int ValidAdddress(LinkedList_t *node, ptr_t ptr){
    if(node == NULL) return 0;

    if(node->next == NULL) return 0;

    if(ptr.addr >= node->start.addr + node->start.size && ptr.addr < node->next->start.addr){
        if(ptr.size <= ((node->start.addr + node->start.size) + (node->next->start.addr - (node->start.addr + node->start.size)) - ptr.addr)) return 1;
        return 0;
    }

    if(ValidAdddress(node->next, ptr)) return 1;

    return 0;
}
//Liberar toda la lista
void FreeList(LinkedList_t *node){
    if(node == NULL) return;

    FreeList(node->next);
    free(node);
}