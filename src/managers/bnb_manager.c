#include "bnb_manager.h"
#include <limits.h>
#include "stdio.h"

//creamos una lista con todos los procesos
static linked_node_t* procs_list;
//almacenamos el pid proceso actual
static process_t current_procs;
//bound de cada proceso
static int bound;
//espacios libres en la memoria real
static free_list_t* free_memory;

//anadimos un espacio libre a la lista
//segun el caso hacemos fusion con el nodo anterior, con el nodo siguiente o con ambos
void free_insert(free_list_t** head, addr_t addr, int size){
  free_list_t* new_node_free = (free_list_t*) malloc(sizeof(free_list_t));
  free_space_t new_data_free = {addr,size};
  if (*head == NULL){
    new_node_free->data = new_data_free;
    new_node_free->prev=NULL;
    new_node_free->next=NULL;
    (*head) = new_node_free;
    return;
  }
  free_list_t* aux = *head;
  free_list_t* predecesor = NULL;
  while(aux != NULL){
    if(aux->data.addr <= addr ){
      predecesor = aux;
    }
    aux = aux->next;
  }
  if(predecesor == NULL){
    new_node_free->data = new_data_free;
    new_node_free->next = (*head);
    (*head)->prev = new_node_free;
    (*head) = new_node_free;
    return;
  }
  if(predecesor->data.addr + predecesor->data.size == addr){
    predecesor->data.size = predecesor->data.size + size;
    if(predecesor->next != NULL && predecesor->data.addr + predecesor->data.size == (predecesor->next)->data.addr){
      predecesor->data.size = predecesor->data.size+ (predecesor->next)->data.size;
      if((predecesor->next)->next != NULL){
        predecesor->next = (predecesor->next)->next;
        free((predecesor->next)->prev);
        (predecesor->next)->prev = predecesor;
      }
      else{
        free(predecesor->next);
        predecesor->next = NULL;
      }
    }
    return;
  }

  new_node_free->data = new_data_free;
  new_node_free->prev = predecesor;
  new_node_free->next = predecesor->next;
  predecesor->next = new_node_free;
  if(new_node_free->next != NULL){
    (new_node_free->next)->prev = new_node_free;
  }
  if(new_node_free->next != NULL && new_node_free->data.addr + new_node_free->data.size == (new_node_free->next)->data.addr){
    new_node_free->data.size = new_node_free->data.size+ (new_node_free->next)->data.size;
    if((new_node_free->next)->next != NULL){
        new_node_free->next = (new_node_free->next)->next;
        free((new_node_free->next)->prev);
        (new_node_free->next)->prev = new_node_free;
      }
      else{
        free(new_node_free->next);
        new_node_free->next = NULL;
      }
  }
  return;
}

//eliminamos un espacio libre y devolvemos el addr del inicio de donde hemos cogido espacio
addr_t free_delete(free_list_t** head, int size){
  free_list_t* aux = *head;
  int min= INT_MAX;
  free_list_t* need_space = NULL;
  addr_t ret;
  while (aux != NULL){
    if(aux->data.size - size >= 0 && aux->data.size - size < min){
      min= aux->data.size - size;
      need_space = aux;
    }
    aux = aux->next;
  }
  if(need_space != NULL){
    if(min!=0){
      need_space->data.addr = need_space->data.addr + size;
      need_space->data.size = need_space->data.size - size;
      return need_space->data.addr - size;
    }
    if(need_space->prev != NULL){
      (need_space->prev)->next = need_space->next;
    }
    if(need_space->next != NULL){
      (need_space->next)->prev = need_space->prev;
    }

    if(need_space == *head){
      (*head)=need_space->next;
    }
    ret = need_space->data.addr;
    free(need_space);
    return ret - size;
  }
  return -1;
}

//guardamos un espacio en el stack
int push_stack(free_list_t** head, addr_t* stack_pointer){
  free_list_t* aux = *head;
  //verificamos que la lista no sea null
  if(aux == NULL){
    return 1;
  }
  //guardamos el ultimo elemento
  while(aux->next != NULL){
    aux = aux->next;
  }
  //actualizamos el valor del stack pointer
  if(aux->data.addr + aux->data.size-1 == *stack_pointer){
    aux->data.size--;
    *stack_pointer= *stack_pointer-1;
    if(aux->data.size== 0){
      (aux->prev)->next = NULL;
      //liberamos el espacio
      free(aux);
    }
    return 0;
  }
  return 1;
}

//eliminamos un espacio del stack
int pop_stack(free_list_t** head, addr_t* stack_pointer, byte *out, procs_info_t curr_procs){
  //verificamos que sea valida el addr que queremos eliminar
  if((int)*stack_pointer != (int)curr_procs.base + bound-1){
    //leemos de la memoria y guardamos en out
    *out = m_read(*stack_pointer+curr_procs.base);
    //insertamos ese espacio libre
    free_insert(head,*stack_pointer,1);
    //actualizamos el stack pointer
    *stack_pointer=*stack_pointer+1;
    return 0;
  }
  return 1;
}

//inserta al inicio de una linked list de procesos
void linked_insert (linked_node_t** head, procs_info_t procs){
    linked_node_t* new_node = (linked_node_t*) malloc(sizeof(linked_node_t));
    new_node->data = procs;
    new_node->prev_node = NULL;
    if(*head == NULL){
      new_node->next_node = NULL;
      (*head) = new_node;
      return;
    }
    new_node->next_node = (*head);
    (*head)->prev_node = new_node;
    (*head) = new_node;
}

//elimina el proceso que se pide segun su pid en una linked list de procesos
void linked_delete(linked_node_t** head, int pid){
  linked_node_t* aux = *head;
  while(aux != NULL){
    if(aux->data.data.pid == pid){
      if(aux->prev_node != NULL){
        aux->prev_node->next_node = aux->next_node;
      }
      if(aux->next_node != NULL){
        aux->next_node->prev_node = aux->prev_node;
      }
      if(aux == *head){
        (*head) = aux->next_node;
      }
      free(aux);
    }
    aux = aux->next_node;
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  procs_list = NULL;
  free_memory = NULL;
  free_insert(&free_memory,0,m_size());
  current_procs.pid=-1;
  current_procs.program=NULL;
  bound = 800;
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      //buscamos un espacio en la free list
      addr_t aux1 = free_delete(&aux->data.head,size);
      if((int)aux1 == -1){
        return 1;
      }
      //y lo almacenamos en out
      out->addr = aux1;
      return 0;
    }
    aux = aux->next_node;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      if(ptr.addr > aux->data.data.program->size-1 && ptr.addr + ptr.size < aux->data.stack_pointer){
        //liberamos el espacio de la free list de icho proceso
        free_insert(&aux->data.head, ptr.addr, ptr.size);
        return 0;
      }
      return 1;
    }
    aux=aux->next_node;
  }
  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      //guardamos un espacio en el stack
      int valid = push_stack(&aux->data.head, &aux->data.stack_pointer);
      if(valid == 0){
        //si pudimos guardar el espacio almacenamos el valor del stack pointer
        //y lo escribimos en la memoria
        out->addr = aux->data.stack_pointer;
        m_write(aux->data.stack_pointer + aux->data.base, val);
        return 0;
      }
      return 1;
    }
    aux=aux->next_node;
  }
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      //leemos en la memoria real guardamos y guardamos en out, liberamos el espacio que ocupaba
      return pop_stack(&aux->data.head, &aux->data.stack_pointer, out,aux->data);
    }
    aux = aux->next_node;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      //verficamos que el addr sea valido
      free_list_t *aux1 = aux->data.head;
      while(aux1 != NULL){
        if(addr >= aux1->data.addr && addr <= aux1->data.addr + aux1->data.size) return 1;
        aux1 = aux1->next;
      }
      //leemos de la memoria y guardamos en out
      *out = m_read(addr + aux->data.base);
      return 0;
    }
    aux = aux->next_node;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  //buscamos el proceso actual
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      //verificamos que el addr sea valido
      free_list_t *aux1 = aux->data.head;
      while(aux1 != NULL){
        if(addr >= aux1->data.addr && addr <= aux1->data.addr + aux1->data.size) return 1;
        aux1 = aux1->next;
      }
      //escribimos en la memoria
      m_write(addr+aux->data.base, val);\
      return 0;
    }
    aux = aux->next_node;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  //buscamos si el proceso ya fue almacenado 
  int bool = 0;
  linked_node_t* aux = procs_list;
  while (aux != NULL){
    if(process.pid == aux->data.data.pid){
      bool = 1;
      break;
    }
    aux = aux->next_node;
  }
  if(bool == 1){
    current_procs = process;
    return;
  }
    //si no esta, hay un cambio de contexto, iniciamos un proceso nuevo y le ponemos todas sus propiedades
    //y actualizamos el current procs
  free_list_t* aux1 = (free_list_t*)malloc(sizeof(free_list_t));
  aux1->data.addr = process.program->size;
  aux1->data.size = bound - process.program->size;
  aux1->next = NULL;
  aux1->prev = NULL;
  addr_t aux2 = free_delete(&free_memory, bound);
  procs_info_t new = {process,aux1,aux2,bound,bound-1};
  linked_insert(&procs_list, new);
  current_procs = process;
  m_set_owner(aux2,aux2+bound-1);
  return;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  //libera todos los espacios ocupados por el proceso
  linked_node_t* aux = procs_list;
  while(aux != NULL){
    if(aux->data.data.pid == current_procs.pid){
      current_procs.pid=-1;
      current_procs.program=NULL;
      free_insert(&free_memory, aux->data.base, aux->data.bounds);
      m_unset_owner(aux->data.base, aux->data.bounds+aux->data.base-1);
      linked_delete(&procs_list, process.pid);
      return;
    }
    aux = aux->next_node;
  }
}
