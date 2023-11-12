#include "bnb_manager.h"
#include "free_list.h"

#include "stdio.h"

#define BLOCK_SIZE 1024
#define STACK_SIZE 64

//estructura con los datos de la memoria utilizada por un proceso
typedef struct bnb_proc_mem_info {
  int pid;//id del proceso
  uint base;  //base
  uint bounds;//bounds
  uint process_bounds;//bounds del proceso
  uint stack_pos;//posición del stack
  uint stack_bounds;//bounds del stack
  uint heap_pos;//posicion del heap
} bnb_proc_mem_info_t;

//estructura de la lista de procesos
typedef struct bnb_proc_mem_info_node {
  bnb_proc_mem_info_t proc_mem_info;//información del proceso
  struct bnb_proc_mem_info_node *next;//próximo proceso
} bnb_proc_mem_info_node_t;

bnb_proc_mem_info_node_t *bnb_proc_mem_info_list_head = NULL; //lista de procesos (header)   
bnb_proc_mem_info_node_t *bnb_current_proccess = NULL;//proceso que se esta ejecutando


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  bnb_proc_mem_info_node_t *temp;
  
  //si queda de un caso anterior, libero la lista de procesos
  while (bnb_proc_mem_info_list_head != NULL) {
      temp = bnb_proc_mem_info_list_head;
      bnb_proc_mem_info_list_head = bnb_proc_mem_info_list_head->next;
      free(temp);
  }
  bnb_current_proccess = NULL;
 
  fl_init(m_size());//inicializar free list
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  //si hay espacio para reservar memoria en el heap
  if (bnb_current_proccess->proc_mem_info.heap_pos + size < bnb_current_proccess->proc_mem_info.stack_pos ){
    out->addr = bnb_current_proccess->proc_mem_info.heap_pos;//reservo
    out->size = size;
    bnb_current_proccess->proc_mem_info.heap_pos += size; //incremento posicion en el heap  
    return 0;
  }
  else
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  //si la direccion esta en el espacio de direcciones del heap
  if (ptr.addr <=bnb_current_proccess->proc_mem_info.heap_pos){
    if (ptr.addr == bnb_current_proccess->proc_mem_info.heap_pos - ptr.size)
      bnb_current_proccess->proc_mem_info.heap_pos -= ptr.size;
    return 0;
  }
  else
    return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  //si hay espacio libre en el stack
  if (bnb_current_proccess->proc_mem_info.stack_pos - 1 > bnb_current_proccess->proc_mem_info.bounds - bnb_current_proccess->proc_mem_info.stack_bounds && bnb_current_proccess->proc_mem_info.stack_pos - 1 > bnb_current_proccess->proc_mem_info.heap_pos){
    bnb_current_proccess->proc_mem_info.stack_pos -= 1;//decremento la posicion en el stack
    m_write(bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.stack_pos, val);//escribo el valor
    out->addr =  bnb_current_proccess->proc_mem_info.stack_pos;//retorno direccion
    out->size = 1;
    return 0;
  }
  else
    return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  //si no me salgo del espacio del stack
  if (bnb_current_proccess->proc_mem_info.stack_pos + 1 <= bnb_current_proccess->proc_mem_info.bounds){
    *out = m_read(bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.stack_pos);//leo el valor
    bnb_current_proccess->proc_mem_info.stack_pos += 1;//incremento la posicion en el stack
    return 0;
  }
  else
    return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //si la direccion esta en el espacio del heap
  if (bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.process_bounds+ addr <= bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.bounds){
    *out = m_read(bnb_current_proccess->proc_mem_info.base + addr);//leo el valor
    return 0;
  }
  else
    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  //si la direccion esta en el espacio del heap
  if (bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.process_bounds + addr <= bnb_current_proccess->proc_mem_info.base + bnb_current_proccess->proc_mem_info.bounds){
    m_write(bnb_current_proccess->proc_mem_info.base + addr, val);//escribo el valor
    return 0;
  }
  else
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  bnb_proc_mem_info_node_t *temp = bnb_proc_mem_info_list_head;

  
  //primero busco si ya el proceso existe
  while (temp != NULL) {
    
    if (temp->proc_mem_info.pid == process.pid) {//si ya existe
      bnb_current_proccess = temp;//lo pongo como proceso actual
      return;    
    }
    temp = temp->next;
  }

  //si no existe el proceso, tengo que crear un nuevo elemento de la lista de procesos
  bnb_current_proccess = (bnb_proc_mem_info_node_t *) malloc(sizeof(bnb_proc_mem_info_node_t));//reservo memoria para info proceso
  bnb_current_proccess->proc_mem_info.pid = process.pid;// id del proceso
  bnb_current_proccess->proc_mem_info.base = fl_alloc(BLOCK_SIZE);//reservo espacio en la free list
  //Comprobar que se reservó bien la memoria para el proceso
  if ((int)bnb_current_proccess->proc_mem_info.base ==-1){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(bnb_current_proccess);
    bnb_current_proccess = NULL;
    return;
  }
  bnb_current_proccess->proc_mem_info.bounds = BLOCK_SIZE;
  bnb_current_proccess->proc_mem_info.stack_pos = bnb_current_proccess->proc_mem_info.bounds;
  bnb_current_proccess->proc_mem_info.stack_bounds = STACK_SIZE;
  bnb_current_proccess->proc_mem_info.process_bounds = process.program->size;
  bnb_current_proccess->proc_mem_info.heap_pos= round_proccess_size(process.program->size);
   //Adiciono el proceso a la lista de procesos
  bnb_current_proccess->next = bnb_proc_mem_info_list_head;
  bnb_proc_mem_info_list_head = bnb_current_proccess;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  bnb_proc_mem_info_node_t *temp = bnb_proc_mem_info_list_head;
  bnb_proc_mem_info_node_t *previus = NULL;

  // busco el proceso actual en mi lista de procesos
  while (temp != NULL) {
    if (temp->proc_mem_info.pid == process.pid) {
      if (previus == NULL)
        bnb_proc_mem_info_list_head = temp->next;
      else 
        previus->next = temp->next;
      
      //libero en el free list la memoria
      fl_free(temp->proc_mem_info.base, temp->proc_mem_info.base + temp->proc_mem_info.bounds-1);

      free(temp);//libero memoria
      
      return;    
    }
    previus = temp;
    temp = temp->next;
  }
  
}
