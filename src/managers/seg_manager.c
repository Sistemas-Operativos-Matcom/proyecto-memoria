#include "seg_manager.h"
#include "free_list.h"
#include "stdio.h"

#define STACK_SIZE 64
#define HEAP_SIZE 512
//256

//estructura con los datos de la memoria utilizada por un proceso
typedef struct seg_proc_mem_info {
  int pid;//id del proceso
  uint proccess_base;//dirección base del proceso
  uint proccess_bounds;//bound del proceso
  uint heap_base;  //dirección base del heap
  uint heap_bounds;//bound del heap
  uint heap_pos;//posicion en el heap
  uint stack_base;  //dirección base del stack
  uint stack_bounds;//bound del stack
  uint stack_pos;//posicion en el stack
} seg_proc_mem_info_t;

//estructura de la lista de procesos
typedef struct seg_proc_mem_info_node {
  seg_proc_mem_info_t proc_mem_info;//información del proceso
  struct seg_proc_mem_info_node *next;//próximo proceso
} seg_proc_mem_info_node_t;

seg_proc_mem_info_node_t *seg_proc_mem_info_list_head = NULL;//lista de procesos (header)  
seg_proc_mem_info_node_t *seg_current_proccess = NULL;//proceso que se esta ejecutando

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) {
  seg_proc_mem_info_node_t *temp;

  //si queda de un caso anterior, libero la lista de procesos
  while (seg_proc_mem_info_list_head != NULL) {
      temp = seg_proc_mem_info_list_head;
      seg_proc_mem_info_list_head = seg_proc_mem_info_list_head->next;
      free(temp);
  }
  seg_current_proccess = NULL;
  
  fl_init(m_size());//inicializar free list

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {
  //si hay espacio para reservar memoria en el heap
  if (seg_current_proccess->proc_mem_info.heap_pos + size < seg_current_proccess->proc_mem_info.heap_bounds){
    out->addr = seg_current_proccess->proc_mem_info.heap_pos;//reservo
    out->size = size;
    seg_current_proccess->proc_mem_info.heap_pos += size + 1; //incremento posicion en el heap 
    return 0;
  }
  else
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {
  //si la direccion esta en el espacio de direcciones del heap
  if (ptr.addr <=seg_current_proccess->proc_mem_info.heap_pos){
    if (ptr.addr == seg_current_proccess->proc_mem_info.heap_pos - ptr.size - 1)
      seg_current_proccess->proc_mem_info.heap_pos -= ptr.size;
    return 0;
  }
  else
    return 1;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {
  //si hay espacio libre en el stack
  if (seg_current_proccess->proc_mem_info.stack_pos - 1 > 0){
    seg_current_proccess->proc_mem_info.stack_pos -= 1;//decremento la posicion en el stack
    m_write(seg_current_proccess->proc_mem_info.stack_base + seg_current_proccess->proc_mem_info.stack_pos, val);//escribo el valor
    out->addr =  seg_current_proccess->proc_mem_info.stack_pos;//retorno direccion
    out->size = 1;
    return 0;
  }
  else
    return 1;
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {
  //si no me salgo del espacio del stack
  if (seg_current_proccess->proc_mem_info.stack_pos + 1 <= seg_current_proccess->proc_mem_info.stack_bounds){
    *out = m_read(seg_current_proccess->proc_mem_info.stack_base + seg_current_proccess->proc_mem_info.stack_pos);//leo el valor
    seg_current_proccess->proc_mem_info.stack_pos += 1;//incremento la posicion en el stack
    return 0;
  }
  else
    return 1;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {
  //si la direccion esta en el espacio del heap
  if (addr <= seg_current_proccess->proc_mem_info.heap_bounds){
    *out = m_read(seg_current_proccess->proc_mem_info.heap_base + addr);//leo el valor
    return 0;
  }
  else
    return 1;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {
  //si la direccion esta en el espacio del heap
  if (addr <= seg_current_proccess->proc_mem_info.heap_bounds){
    m_write(seg_current_proccess->proc_mem_info.heap_base + addr, val);//escribo el valor
    return 0;
  }
  else
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {
  seg_proc_mem_info_node_t *temp = seg_proc_mem_info_list_head;

  //primero busco si ya el proceso existe
  while (temp != NULL) {
    if (temp->proc_mem_info.pid == process.pid) {//si ya existe
      seg_current_proccess = temp;//lo pongo como proceso actual
      return;    
    }
    temp = temp->next;
  }

  //si no existe el proceso, tengo que crear un nuevo elemento de la lista de procesos
  seg_current_proccess = (seg_proc_mem_info_node_t *) malloc(sizeof(seg_proc_mem_info_node_t));//reservo memoria para info proceso
  seg_current_proccess->proc_mem_info.pid = process.pid;// id del proceso
  
  seg_current_proccess->proc_mem_info.proccess_base = fl_alloc(round_proccess_size(process.program->size));//reservo espacio para el codigo del proceso en la free list
  //Comprobar que se reservó bien la memoria para el proceso
  if ((int)seg_current_proccess->proc_mem_info.proccess_base ==-1){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(seg_current_proccess);
    seg_current_proccess = NULL;
    return;
  }
  seg_current_proccess->proc_mem_info.proccess_bounds = round_proccess_size(process.program->size);
    

  seg_current_proccess->proc_mem_info.heap_base = fl_alloc( HEAP_SIZE);//reservo espacio para el heap
  //Comprobar que se reservó bien la memoria para el heap
  if ((int)seg_current_proccess->proc_mem_info.heap_base ==-1){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(seg_current_proccess);
    seg_current_proccess = NULL;
    return;
  }
  seg_current_proccess->proc_mem_info.heap_bounds = HEAP_SIZE;
  
  seg_current_proccess->proc_mem_info.heap_pos = 0;
  
  
  seg_current_proccess->proc_mem_info.stack_base = fl_alloc(STACK_SIZE);//reservo espacio para el stack
  //Comprobar que se reservó bien la memoria para el stack
  if ((int)seg_current_proccess->proc_mem_info.stack_base ==-1){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(seg_current_proccess);
    seg_current_proccess = NULL;
    return;
  }
  seg_current_proccess->proc_mem_info.stack_bounds = STACK_SIZE;
  
  seg_current_proccess->proc_mem_info.stack_pos = seg_current_proccess->proc_mem_info.stack_bounds;
  //Adiciono el proceso a la lista de procesos
  seg_current_proccess->next = seg_proc_mem_info_list_head;
  seg_proc_mem_info_list_head = seg_current_proccess;

}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  seg_proc_mem_info_node_t *temp = seg_proc_mem_info_list_head;
  seg_proc_mem_info_node_t *previus = NULL;

  // busco el proceso actual en mi lista de procesos
  while (temp != NULL) {
    if (temp->proc_mem_info.pid == process.pid) {
      if (previus == NULL)
        seg_proc_mem_info_list_head = temp->next;
      else 
        previus->next = temp->next;

      
      //libero en el free list la memoria de código, heap y stack utilizada
      fl_free(temp->proc_mem_info.proccess_base, temp->proc_mem_info.proccess_base + temp->proc_mem_info.proccess_bounds-1);
      fl_free(temp->proc_mem_info.heap_base, temp->proc_mem_info.heap_base + temp->proc_mem_info.heap_bounds-1);
      fl_free(temp->proc_mem_info.stack_base, temp->proc_mem_info.stack_base + temp->proc_mem_info.stack_bounds-1);
      
      free(temp);//libero memoria
      return;    
    }
    previus = temp;
    temp = temp->next;
  }
}
