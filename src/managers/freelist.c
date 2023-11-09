#include "freelist.h"
#include "stdlib.h"
#include "stdio.h"

struct Freelist* freelist_init(size_t heap_begining, size_t heap_initial_size){
    struct Freelist* freelist = (struct Freelist*) malloc(sizeof(struct Freelist));
    freelist->heap_begining = heap_begining;
    freelist->heap_size = heap_initial_size;
    freelist->first = (struct FreelistNode*) malloc(sizeof(struct FreelistNode));

    freelist->first->next = NULL;
    freelist->first->begining = heap_begining;
    freelist->first->size = heap_initial_size;

    return freelist;
}

int freelist_alloc_space(struct Freelist* freelist, size_t size, size_t* out_addr){
    struct FreelistNode* current_node = freelist->first;
    struct FreelistNode* prev_node = NULL;
    
    while(current_node != NULL){
        if(current_node->size >= size){
            *out_addr = current_node->begining+current_node->size-size;
            current_node->size -= size;
            
            if (current_node->size == 0){
                if(prev_node == NULL){
                    freelist->first = current_node->next;
                }else{
                    prev_node->next = current_node->next;
                }
                free(current_node);
            }

            return 0;
        }
        prev_node = current_node;
        current_node = current_node->next;
    }
    return -1;
}

// Cuenta cuantos nodos hay en la freelist (para debug)
int debug_freelist_node_count(struct Freelist* freelist){
    int count = 0;
    struct FreelistNode* current = freelist->first;
    while(current != NULL){
        count += 1;
        current = current->next;
    }
    return count;
}

void freelist_try_merge_nodes(struct FreelistNode* from){
    
    struct FreelistNode* middle = from->next;
    if(from != NULL && from->next != NULL && from->begining + from->size == from->next->begining){
        from->size += from->next->size;
        struct FreelistNode* tmp = from->next;
        from->next = from->next->next;
        free(tmp);
        middle = from;
    }

    if(middle != NULL && middle->next != NULL && middle->begining + middle->size == middle->next->begining){
        middle->size += middle->next->size;
        struct FreelistNode* tmp = middle->next;
        middle->next = middle->next->next;
        free(tmp);
    }

}


void freelist_free_space(struct Freelist* freelist, size_t begining, size_t size){
    struct FreelistNode* new_node = (struct FreelistNode*) malloc(sizeof(struct FreelistNode));
    new_node->begining = begining;
    new_node->size = size;
    new_node->next = NULL;

    //Caso 1: lo que se quiere liberar queda antes de todo lo que esta libre actualmente [queda al principio de la freelist]
    if(freelist->first == NULL || begining < freelist->first->begining){
        new_node->next = freelist->first;
        freelist->first = new_node;
        freelist_try_merge_nodes(freelist->first);
        return;
    }

    struct FreelistNode* current_node = freelist->first->next;
    struct FreelistNode* prev_node = freelist->first;

    //Caso 2: lo que se quiere liberar queda entre dos espacios libres [entre dos nodos de la freelist]
    while(current_node != NULL){
        if(prev_node->begining < begining && begining < current_node->begining){
            new_node->next = prev_node->next;
            prev_node->next = new_node;
            freelist_try_merge_nodes(prev_node);
            return;
        }
        
        prev_node = current_node;
        current_node = current_node->next;
    }

    //Caso 3: lo que se quiere liberar queda despues de todo lo que esta libre [al final de la freelist]
    prev_node->next = new_node;
    freelist_try_merge_nodes(prev_node);
}

void freelist_grow(struct Freelist* freelist, int d_size){
    
    // Crear el nuevo nodo
    struct FreelistNode* new_node = (struct FreelistNode*) malloc(sizeof(struct FreelistNode));
    new_node->size = d_size;
    new_node->begining = freelist->heap_begining + freelist->heap_size;
    new_node->next = NULL;

    // Encontrar el ultimo nodo de la freelist para insertar el nuevo nodo
    struct FreelistNode* current_node = freelist->first;
    if(current_node == NULL){
        freelist->first = new_node;
    }else{
        while(current_node->next != NULL){
            current_node = current_node->next;
        }
        current_node->next = new_node;
        freelist_try_merge_nodes(current_node);
    }

    // Crecer el size del heap
    freelist->heap_size += d_size;
}

void freelist_deinit(struct Freelist* freelist){
    struct FreelistNode* current_node = freelist->first;
    
    while(current_node != NULL){
        struct FreelistNode* tmp = current_node->next;
        free(current_node);
        current_node = tmp;
    }

    free(freelist);
}



