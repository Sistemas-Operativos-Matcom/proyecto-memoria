#include "free_list.h"

#include "stdio.h"
#include "../memory.h"
#include "../utils.h"

//lista de espacios libres
fl_mem_info_node_t *fl_head = NULL;  

//limpia la lista
void fl_clear() {
   fl_mem_info_node_t *temp;

  while (fl_head != NULL) {
      temp = fl_head;
      fl_head = fl_head->next;
      free(temp);
  }
  fl_head = NULL;
}

//inicializa la lista con un elemento que abarca toda la memoria
void fl_init(size_t size) {
  fl_clear();
  fl_head = (fl_mem_info_node_t *) malloc(sizeof(fl_mem_info_node_t));
  fl_head->block_info.addr = 0;
  fl_head->block_info.len = size;
  fl_head->next = NULL;
}

//elimina la fragmentación de los espacios, uniendo espacios libres consecutivos
void fl_defrag(){
  fl_mem_info_node_t *temp;
  fl_mem_info_node_t *aux;
  unsigned long addr;
  unsigned long len;

  for(temp = fl_head; temp != NULL; temp=temp->next) {
    for(aux = temp->next; aux != NULL; aux=aux->next) {
      if (aux->block_info.addr < temp->block_info.addr) {
        addr = temp->block_info.addr;
        len = temp->block_info.len;
        temp->block_info.addr = aux->block_info.addr;
        temp->block_info.len = aux->block_info.len;
        aux->block_info.addr = addr;
        aux->block_info.len = len;
      }
    }
  }

  temp = fl_head;
  while( temp != NULL) {
    if(temp->next != NULL){
      if (temp->block_info.addr + temp->block_info.len == temp->next ->block_info.addr){
        aux = temp->next;
        temp->block_info.len += temp->next->block_info.len;
        temp->next = temp->next->next;

        free(aux);

      }
    }

    temp=temp->next;
  }
}

//reserva un espacio de memoria
addr_t fl_alloc(size_t size) {
  fl_mem_info_node_t *temp = fl_head;
  addr_t addr;
  size_t min_size = m_size();
  fl_mem_info_node_t *min_block = NULL;
  
  //busco con criterio best-fit el bloque libre donde voy a reservar
  while (temp != NULL) {
    if (temp->block_info.len >= size) {
      if (temp->block_info.len - size < min_size ) { 
        min_size = temp->block_info.len- size;
        min_block = temp;
      }
    }    
    temp = temp->next;
  }
  //si lo he encontrado
  if (min_block != NULL) {
    addr = min_block->block_info.addr;//devuelvo la dirección
    min_block->block_info.addr += size;//actualizo el espacio libre
    min_block->block_info.len -= size;
    m_set_owner(addr, addr + size -1);//hago al proceso propietario de la memoria
    fl_defrag();
    return addr;
  }
  
  return -1;
}


//libera un espacio de memoria
void fl_free(size_t from_addr, size_t to_addr) {
  fl_mem_info_node_t *temp = fl_head;

  //creo un nuevo nodo con los datos del espacio libre
  fl_head = (fl_mem_info_node_t *) malloc(sizeof(fl_mem_info_node_t));
  fl_head->block_info.addr = from_addr;
  fl_head->block_info.len = to_addr - from_addr+1;
  fl_head->next = temp;
  // quito la propiedad de la memoria 
  m_unset_owner(from_addr, to_addr);
  fl_defrag();
  if (fl_head->next->next == NULL)
    fl_defrag();
    
}

//mostrar la free list
void fl_show(){
  fl_mem_info_node_t *temp = fl_head;

  while (temp != NULL) {
    printf("Base: %lu Bounds: %lu\n",temp->block_info.addr, temp->block_info.len);
    temp = temp->next;
  }
  
}
