#include "pag_manager.h"
#include <limits.h>
#include "stdio.h"

//espacios libres en la memoria real
static free_list_t* free_memory;
//almacenamos el proceso actual
static process_t current_procs;
//creamos una lista con todos los procesos
static pag_procs_list_t* pag_procs_list;
//tamano de las paginas
int pag_size;
static int pag_max;

//segun el caso hacemos fusion con el nodo anterior, con el nodo siguiente o con ambos
void pag_free_insert(free_list_t** head, addr_t addr, int size){
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

//eliminamos un espacio libre
addr_t pag_free_delete(free_list_t** head, int size){
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

//inserta al inicio de la lista
void pag_ins_procs (pag_procs_list_t ** head, pag_procs_t procs){
    pag_procs_list_t* new_node = (pag_procs_list_t*) malloc(sizeof(pag_procs_list_t));
    new_node->procs = procs;
    new_node->prev_procs = NULL;
    if(*head == NULL){
      new_node->next_procs = NULL;
      (*head) = new_node;
      return;
    }
    new_node->next_procs = (*head);
    (*head)->prev_procs = new_node;
    (*head) = new_node;
}

//elimina el proceso que se pide segun su pid
void pag_del_procs(pag_procs_list_t ** head, int pid){
  pag_procs_list_t* aux = *head;
  while(aux != NULL){
    if(aux->procs.procs.pid == pid){
      if(aux->prev_procs != NULL){
        aux->prev_procs->next_procs= aux->next_procs;
      }
      if(aux->next_procs != NULL){
        aux->next_procs->prev_procs = aux->prev_procs;
      }
      if(aux == *head){
        (*head) = aux->next_procs;
      }
      free(aux);
    }
    aux = aux->next_procs;
  }
  return;
}

//inserta una pagina
void pag_ins_pag(page_list_t** head, page_t pag){
  page_list_t* new_page = (page_list_t*) malloc(sizeof(page_list_t));
  new_page->page = pag;
  new_page->prev_page = NULL;
  if(*head == NULL){
    new_page->next_page = NULL;
    (*head) = new_page;
    return;
  }
  new_page->next_page = (*head);
  (*head)->prev_page = new_page;
  (*head) = new_page;
  return;
}

//elimina una pagina
void pag_del_pag(page_list_t** head, int num){
  page_list_t* aux = *head;
  while(aux != NULL){
    if(aux->page.number == num){
      if(aux->prev_page != NULL){
        (aux->prev_page)->next_page = aux->next_page;
      }
      if(aux->next_page != NULL){
        (aux->next_page)->prev_page = aux->prev_page;
      }
      if(aux == *head){
        (*head) = aux->next_page;
      }
      free(aux);
      return;
    }
    aux = aux->next_page;
  }
  return;
}

//inserta un entero
void int_ins(int_l_t** head, int number){
  int_l_t *new = (int_l_t*)malloc(sizeof(int_l_t));
  new->data = number;
  new->next_int = (*head);
  if(*head == NULL){
    (*head) = new;
    return;
  }
  (*head)->prev_int = new;
  (*head) = new;
  return;
}

//elimina un entero
void int_del(int_l_t** head, int number){
  int_l_t *aux = *head;
  while(aux != NULL){
    if(aux->data == number){
      if(aux->prev_int != NULL){
        (aux->prev_int)->next_int = aux->next_int;
      }
      if(aux->next_int != NULL){
        (aux->next_int)->prev_int = aux->prev_int;
      }
      if(aux == *head){
        (*head) = aux->next_int;
      }
      free(aux);
      return;
    }
    aux = aux->next_int;
  }
  return;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  pag_procs_list = NULL;
  free_memory = NULL;
  pag_free_insert(&free_memory, 0, m_size());
  current_procs.pid = -1;
  current_procs.program = NULL;
  pag_size = 50;
  pag_max = 16;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      int cant = size/pag_size;
      int resto = size%pag_size;
      if(resto !=0) cant +=1;
      //revisamos la cantidad de paginas necesarias para crear
      page_list_t* aux2 = aux->procs.pag_heap;
      while(aux2 != NULL){
        //las guardamos
        addr_t result = pag_free_delete(&aux->procs.pag_heap->page.free_head,size);
        if((int)result != -1){
          out->addr = result + aux2->page.size*aux2->page.number;
          return 0;
        }
        aux2 = aux2->next_page;
      }
      //verificamos que se puedan crear todas las paginas necesarias
      if(aux->procs.pag_max + cant <= pag_max){
        int aux1 = 0;
        addr_t aux_adr = -1;
        while(aux1 < cant){
          //creamos una a una
          //las guardamos
          addr_t adr =  pag_free_delete(&free_memory, pag_size);
          //seteamos owner
          m_set_owner(adr,adr+pag_size-1);
          free_list_t *free_l = NULL;
          if(aux1 ==cant-1 && resto != 0){
            free_space_t data = {resto, pag_size-resto};
            free_l = (free_list_t*)malloc(sizeof(free_list_t));
            free_l->data = data;
          }
          int n = aux->procs.procs_int_list->data;
           if(aux1 == 0){
            aux_adr = (addr_t)n;
           }
           //las creamos y guardamos
          page_t pag = {n,pag_size, free_l,adr};
          int_del(&aux->procs.procs_int_list, n);
          pag_ins_pag(&aux->procs.pag_heap, pag);
          aux->procs.pag_max+=1;
          aux1 +=1;
        }
        out->addr = aux_adr*pag_size;
        return 0;
      }
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      //vemos la cantidad de paginas a liberar
      int search = ptr.addr/pag_size;
      int position = ptr.addr%pag_size;
      int cant_pag = ptr.size/pag_size;
      int ult_pag = ptr.size%pag_size;
      //las eliminamos, le quitamos el owner y ponemos disponibles el numero de cada una
      while(cant_pag >=0){
        page_list_t* aux2 = aux->procs.pag_heap;
        while(aux2 != NULL){
          if(aux2->page.number == search){
            if(cant_pag == 0 && position != 0){
              pag_free_insert(&aux2->page.free_head, 0, position);
              if(aux2->page.free_head->data.addr == 0 && (int)aux2->page.free_head->data.addr == pag_size){
                int_ins(&aux->procs.procs_int_list, aux2->page.number);
                m_unset_owner(aux->procs.pag_heap->page.real_addr,aux->procs.pag_heap->page.real_addr +pag_size-1);
                pag_del_pag(&aux->procs.pag_heap, aux2->page.number);
              }
              return 0;
            }
            else if(cant_pag == 0 && position != 0){
              return 0;
            }

            if(aux2->prev_page != NULL) search = aux2->prev_page->page.number;
            cant_pag -=1;
            int_ins(&aux->procs.procs_int_list, aux2->page.number);
            m_unset_owner(aux->procs.pag_heap->page.real_addr, aux->procs.pag_heap->page.real_addr+pag_size-1);
            pag_del_pag(&aux->procs.pag_heap, aux2->page.number);
          }
          aux2 = aux2->next_page;
        }
      }
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      //si el stack no tiene paginas o la ultima pagina del stack esta llena
      if(aux->procs.pag_stack == NULL || (int)aux->procs.stack_pointer == pag_size-1){
        if(aux->procs.pag_max < pag_max){
          //creamos una pagina nueva, le seteamos el owner, y le ponemos todas las propiedades
          addr_t adr =  pag_free_delete(&free_memory, pag_size);
          m_set_owner(adr,adr+pag_size-1);
          free_list_t *free_l = NULL;
          pag_free_insert(&free_l, 1, pag_size-1);
          int n = aux->procs.procs_int_list->data;
          int_del(&aux->procs.procs_int_list, n);
          page_t new_p = {n,pag_size,free_l,adr};
          pag_ins_pag(&aux->procs.pag_stack, new_p);
          aux->procs.pag_max+=1;
          //escribimos en memoria real
          m_write(adr, val);
          //actualizamos stack pointer
          aux->procs.stack_pointer =0;
          //guardamos el valor en out
          out->addr = n*pag_size;
          return 0;
        }
        return 1;
      }
      //si cabe en la pagina actual
      //almacenamos un espacio, actualizamos stack pointer
      addr_t adr =  pag_free_delete(&aux->procs.pag_stack->page.free_head, 1);
      aux->procs.stack_pointer +=1;
      //escribimos en la memoria real
      m_write(aux->procs.pag_stack->page.real_addr + adr, val);
      //almacenamos el valor en out
      out->addr =aux->procs.pag_stack->page.number*pag_size+adr;
      return 0;
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      //si tiene paginas del stack
      if(aux->procs.stack_pointer>0){
        //liberamos el espacio
        pag_free_insert(&aux->procs.pag_stack->page.free_head, aux->procs.stack_pointer, 1);
        //almacenamos el valor en out
        *out = m_read(aux->procs.pag_stack->page.real_addr + aux->procs.stack_pointer);
        //actualizamos stackk pointer
        aux->procs.stack_pointer-=1;
        return 0;
      }
      //si al mover el stack tenemos que eliminar una pagina
      //leemos de memoriam, quitamos el owner, ponemos disponible el entero de la pagina
      *out = m_read(aux->procs.pag_stack->page.real_addr + aux->procs.stack_pointer);
      m_unset_owner(aux->procs.pag_stack->page.real_addr + aux->procs.stack_pointer, aux->procs.pag_stack->page.real_addr + aux->procs.stack_pointer+pag_size-1);
      aux->procs.stack_pointer=pag_size-1;
      int_ins(&aux->procs.procs_int_list, aux->procs.pag_stack->page.number);
      //eliminamos la pagina de la lista
      pag_del_pag(&aux->procs.pag_stack, aux->procs.pag_stack->page.number);
      //actualizamos la cantidad de paginas del proceso
      aux->procs.pag_max -=1;
      //actualizamos stack pointer
      if(aux->procs.pag_stack == NULL) aux->procs.stack_pointer = -1;
      return 0;
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      page_list_t* aux2= aux->procs.pag_heap;
      //buscamos la pagina que tenemos que cargar
      while(aux2 != NULL){
        if(aux2->page.number == (int)addr/pag_size){
          //leemos de la memoria real y almacenamos en out
          *out = m_read(aux2->page.real_addr + addr%pag_size);
          return 0;
        }
        aux2= aux2->next_page;
      }
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      page_list_t* aux2= aux->procs.pag_heap;
      //buscamos la pagina en que nos piden guardar informacion
      while(aux2 != NULL){
        if(aux2->page.number == (int)addr/pag_size){
          //guardamos en memoria real
          m_write(aux2->page.real_addr + addr%pag_size,val);
          return 0;
        }
        aux2= aux2->next_page;
      }
    }
    aux = aux->next_procs;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  //verificamos si el proceso ya estaba en la lista de procesos
  int bool = 0;
  pag_procs_list_t* aux =  pag_procs_list;
  while (aux != NULL){
    if(process.pid == aux->procs.procs.pid){
      bool = 1;
      break;
    }
    aux = aux->next_procs;
  }
  if(bool == 1){
    //si es asi solo actualizamos curren procs
    current_procs = process;
    return;
  }
  //si no lo tenemos, tenemos que crear un proceso nuevo y ponerle todas las propiedades

  //creamos una lista de paginas para el codigo, heap y stack
  //como solo estamos iniciando solo rellenamos las del codigo
  page_list_t *pag_heap = NULL;
  page_list_t *pag_stack = NULL;
  page_list_t *pag_cod = NULL;

  //rellenamos la lista de paginas del codigo segun la cantidad de datos
  int cant = process.program->size/pag_size;
  int resto = process.program->size%pag_size;
  if(resto !=0) cant +=1;
  int a = 0;
  while(a < cant){
    addr_t adr =  pag_free_delete(&free_memory, pag_size);
    //seteamos owner de cada pagina
    m_set_owner(adr,adr+pag_size-1);
    free_list_t *free_l = NULL;
    if(a ==cant-1 && resto != 0){
      free_space_t data = {resto, pag_size-resto};
      free_l = (free_list_t*)malloc(sizeof(free_list_t));
      free_l->data = data;
    }
    //creamos la pagina
    page_t pag = {a,pag_size, free_l ,adr};
    pag_ins_pag(&pag_cod, pag);
    a +=1;
  }
  int_l_t *int_list = NULL;
  a = pag_max-1;
  while(a >= cant ){
    int_ins(&int_list, a);
    a-=1;
  }
  //finalmente creamos la pagina y la insertamos a la lista de paginas del codigo
  pag_procs_t new_procs = {process, -1, pag_heap, pag_stack, pag_cod, cant, int_list};
  pag_ins_procs (&pag_procs_list, new_procs);
  //actualizamos el current procs
  current_procs = process;
  return;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  //buscamos el proceso actual
  pag_procs_list_t* aux = pag_procs_list;
  while(aux != NULL){
    if(aux->procs.procs.pid == current_procs.pid){
      //buscamos todas las paginas del heap y le quitamos el owner
      page_list_t* pags = aux->procs.pag_heap;
      while(pags != NULL){
        m_unset_owner(pags->page.real_addr,pags->page.real_addr+pag_size-1);
        pags= pags->next_page;
      }
      //buscamos todas las paginas del stack y le quitamos el owner
      page_list_t* stck = aux->procs.pag_stack;
      while(stck != NULL){
        m_unset_owner(stck->page.real_addr, stck->page.real_addr+pag_size-1);
        stck= stck->next_page;
      }
      //buscamos todas las paginas del codigo y le quitamos el owner
      page_list_t* cod = aux->procs.pag_cod;
      while(cod != NULL){
        m_unset_owner(cod->page.real_addr, cod->page.real_addr+pag_size-1);
        cod= cod->next_page;
      }
      //hacemos free a todas las listas de paginas
      free(aux->procs.pag_heap);
      free(aux->procs.pag_cod);
      free(aux->procs.pag_stack);
      free(aux->procs.procs_int_list);
      //actualizamos el curret]nt pid a -1
      current_procs.pid = -1;
      //eliminamos el proceso
      pag_del_procs(&aux,current_procs.pid);
      return;
    }
    aux = aux->next_procs;
  }
  return;
}
