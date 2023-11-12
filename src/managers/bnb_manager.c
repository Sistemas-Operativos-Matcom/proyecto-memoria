#include "bnb_manager.h"

#include "stdio.h"


struct node{
  ptr_t pointer;
  struct node  *next;
};

typedef struct free_list{
  struct node *head;
} free_list_t;
// Tipo que representa el espacio de direcciones para cualquier proceso
typedef struct d_space{
  // Puntero para representar la mayor dirección  del heap y su tamaño 
  // addr -> mayor dirección del heap
  // size -> tamaño del heap
  ptr_t heap_ptr; 
  // Free-list para gestionar los espacios libres del heap
  free_list_t free_list_heap;
  // Puntero que representa el stack
  // addr -> la menor dirección que ocupa el stack, es decir, el tope de la pila
  // size -> cantidad de elementos en la pila
  ptr_t stack_pointer;
}d_space_t;
// Tipo que guarda la información de cada proceso ejecutado en memoria con su respectivo espacio
typedef struct owner{
  // pid del proceso 
  int pid;
  // addr -> base del proceso
  // size -> bounds del proceso
  ptr_t bb;
}owner_t;

///////////////////////////////////////////////////////////////////////////
// Implementación de una free-list para gestionar el espacio libre del heap
void sort(free_list_t fl){
  struct node *curr_node = fl.head;
  while(curr_node != NULL){
    if(curr_node->next != NULL && curr_node->pointer.size > curr_node->next->pointer.size){
      struct node *previous = curr_node;
      curr_node->pointer.addr = curr_node->next->pointer.addr;
      curr_node->pointer.size = curr_node->next->pointer.size;
      curr_node->next = previous;

      previous->next = previous->next->next;
      break;
    }
    curr_node = curr_node->next;
  }
}
int fusion(free_list_t fl){
  if(fl.head->next == NULL){
    return 1;
  }
  struct node *head_copy = fl.head;
  struct node *curr_node = fl.head->next;
  while(curr_node != NULL){
    size_t next_addr = head_copy->pointer.addr + head_copy->pointer.size;
    if(curr_node->next->pointer.addr == next_addr){
      curr_node->pointer.addr = head_copy->pointer.addr;
      curr_node->pointer.size += head_copy->pointer.size;
    }
    curr_node = curr_node->next;
  }
  if(curr_node == NULL){
    return 1;
  }
  return 0;
}
// Dado una dirección y un tamaño, verifica si es un elemento de la free-list
// Devuelve 1 si es un elemento de la free-list; 0 en caso contrario
int is_free(free_list_t fl, addr_t addr, size_t size){
  if(fl.head->pointer.size == 0)
  {
    return 0;
  }else if(fl.head->next == NULL){
    return 0;
  }
  struct node *curr_node = fl.head->next;
  while(curr_node != NULL){
    if(curr_node->pointer.size == size && curr_node->pointer.addr == addr){
      return 1;
    }
    curr_node = curr_node->next;
  }
  return 0;
}
// Dada una dirección y un tamaño, inserta en la free-list un elemento en la cabecera
int insert_at_start(free_list_t fl, addr_t addr, size_t size){
  struct node *elment = (struct node *)malloc(sizeof(struct node)); 
  elment->pointer.addr = addr;
  elment->pointer.size = size;
  elment->next = fl.head;
  fl.head = elment;
  // ordena la free-list en orden ascendente
  sort(fl);
  // si hay dos nodos en la free-list cuyas direcciones son contiguas; crea un solo nodo que encapsula a los mismos
  fusion(fl);
  
  return 0;
}
int delete_node(free_list_t fl, addr_t addr){
  if(fl.head->pointer.addr == addr && fl.head->next == NULL){
    struct node *old_head = fl.head;    
    fl.head = NULL;
    free(old_head);
    return 0;    
  } else if(fl.head->pointer.addr == addr){
    struct node *old_head = fl.head;
    fl.head = fl.head->next;
    free(old_head);
    return 0;
  }
  struct node *curr_node = fl.head->next;
  struct node *previous = fl.head;
  while(curr_node != NULL){
    if(curr_node->pointer.addr == addr){
      previous->next = curr_node->next;
      free(curr_node);
      return 1;
    }
    previous = curr_node;
    curr_node = curr_node->next;
  }
  return 0;
}

// devuelve 1 si la operación se realiza correctamente, 0 en caso contrario
int division(free_list_t fl, size_t size, ptr_t *out){
  if(fl.head->pointer.size == size && fl.head->next == NULL){
    out->addr = fl.head->pointer.addr;
    ///// CUIDADO CON ESTA LINEA DE CODIGO COMO AFECTA A LA OPERACION INSERTAR
    fl.head->pointer.addr = 0;
    fl.head->pointer.size = 0;
    return 1;
  }else if(fl.head->pointer.size > size){
    out->addr = fl.head->pointer.addr;
    fl.head->pointer.addr += size;
    fl.head->pointer.size -= size;
    return 1;
  }
  struct node *curr_node = fl.head->next;
  while(curr_node != NULL){
    if(curr_node->pointer.size == size){
      out->addr = curr_node->pointer.addr;
      delete_node(fl, curr_node->pointer.addr);
      return 1;
    }else if(curr_node->pointer.size > size){
      out->addr = curr_node->pointer.addr;
      curr_node->pointer.addr += size;
      curr_node->pointer.size -= size;
      return 1;
    }
    curr_node = curr_node->next;
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////


// Variables globales
owner_t* owners;
d_space_t* owners_space;
int curr_proc_pid;
int owners_length;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  owners_length = (m_size())/1024;
  owners = (owner_t*)malloc(owners_length * sizeof(owner_t));
  size_t addr = 0;

  for(int i = 0; i < owners_length; i++){
    owners[i].pid = -1;
    owners[i].bb.addr = addr;
    owners[i].bb.size = 1024;
    addr += 1024;
  }
  // Inicializar cada espacio de direcciones
  owners_space = (d_space_t*)malloc(owners_length* sizeof(d_space_t));
  // al inicio de la simulación ningún proceso se ha ejecutado
  curr_proc_pid = -1;  
}
// Dado el pid del proceso actual, busca su posición en la lista owners
int curr_proc_pos(int pid, owner_t* owner){
  if(pid == -1){
    return -1;
  }  
  for(int i = 0; i < owners_length; i++){
    if(owners[i].pid == curr_proc_pid){
      return i;
    }
  }
  return -1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
   FILE* prueba;
           prueba = fopen("test01.txt","a");
           fprintf(prueba,"after switch");
           fclose(prueba);
  
  // asume que el heap no tiene espacios libres
  if(owners_space[pos].heap_ptr.addr + 1 == owners_space[pos].stack_pointer.addr){
    // printf("Conflicto entre Heap y Stack");
    return 1;
  }
  // Verificar si hay un espacio libre en el heap
  if(owners_space[pos].free_list_heap.head->pointer.size != 0){
    // se encontró un espacio libre para reservar memoria
    if(division(owners_space[pos].free_list_heap, size, out))
    {
      // conversión a direccion física de la memoria
      out->addr += owners[pos].bb.addr;  //en la función division se le asigna al puntero la dirección virtual 
      return 0;
    }    
  }else {
    // El Heap no tiene espacio libre o 
    // no hay un espacio libre con tamaño >= size o
    // tiene tamaño 0

    // Aumentar la dirección del puntero y el tamaño del heap
    owners_space[pos].heap_ptr.addr +=1;
    owners_space[pos].heap_ptr.size +=1;
    
    // conversión a dirección física de la memoria
    addr_t pa = owners[pos].bb.addr + owners_space[pos].heap_ptr.addr;    

    out->addr = pa;
  }
  
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
  // si el espacio de memoria ya se ha liberado antes
  if(is_free(owners_space[pos].free_list_heap, ptr.addr, ptr.size)){
    //printf("Error, free() por segunda vez al mismo puntero");
    return 1;
  }
  insert_at_start(owners_space[pos].free_list_heap, ptr.addr, ptr.size);
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
  if(owners_space[pos].stack_pointer.addr - 1 == owners_space[pos].heap_ptr.addr){
    //printf("Conflicto entre Stack y Heap");
    return 1;
  }
  owners_space[pos].stack_pointer.addr -= 1;
  owners_space[pos].stack_pointer.size += 1;

  // conversión a dirección física
  addr_t pa = owners[pos].bb.addr + owners_space[pos].stack_pointer.addr;
  out->addr = pa;
  // escribir en la memoria el valor val en la dirección del puntero out
  m_write(out->addr,val);
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
  if(owners_space[pos].stack_pointer.size == 0){
    //printf("Stack vacóo, operación inválida");
    return 1;
  }
  // conversión a dirección física
  addr_t pa = owners[pos].bb.addr + owners_space[pos].stack_pointer.addr;
  byte val = m_read(pa);
  out = val;  

  // actualizar la dirección del stack pointer
  owners_space[pos].stack_pointer.addr += 1;
  owners_space[pos].stack_pointer.size -=1;

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
  // la dirección virtual addr es mayor que el bounds para el espacio de direcciones
  if(addr > (addr_t)owners[pos].bb.size){
    printf("Intento de acceder a una dirección fuera del espacio de direcciones");
    return 1;
  }
  // conversión a dirección física
  addr_t pa = owners[pos].bb.addr + addr;
  // almacena el valor guardado en la dirección de memoria `pa`
  *out = m_read(pa);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  int pos = curr_proc_pos(curr_proc_pid, owners);
  // la dirección virtual addr es mayor que el bounds para el espacio de direcciones
  if(addr > (addr_t)owners[pos].bb.size){
    // printf("Intento de acceder a una dirección fuera del espacio de direcciones");
    return 1;
  }else if(addr > owners_space[pos].heap_ptr.addr){
    // printf("Dirección fuera del límite del heap");
    return 1;
  }

  // conversión a direccion física
  addr_t pa = owners[pos].bb.addr + addr;
  // guardar el valor en la dirección `pa` de la memoria
  m_write(pa, val);
  
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  // actualiza el valor del pid del proceso en ejecución
  curr_proc_pid = process.pid;
  // actualizar el owner
  //set_curr_owner(process.pid);
  // verificar si `process` no se encuentra en la lista owners
  
  if(curr_proc_pos(curr_proc_pid, owners) == -1){
   
    for(int i = 0; i < owners_length; i++){
      // en la posición i-ésima no hay un proceso almacenado
      if(owners[i].pid == -1){
        owners[i].pid = curr_proc_pid;

         

        // asignar el espacio de memoria correspondiente al proceso
        // inicializar las variables del espacio de memoria
        owners_space[i].heap_ptr.addr = owners[i].bb.addr + process.program->size;
        owners_space[i].heap_ptr.size = 0;
        owners_space[i].stack_pointer.addr = owners[i].bb.size;
        owners_space[i].stack_pointer.size = 0;

 
        
        owners_space[i].free_list_heap.head = NULL;
           
        // owners_space[i].free_list_heap.head->pointer.size = 0;
        // owners_space[i].free_list_heap.head->next = NULL;

        
        // set owner
        m_set_owner(owners[i].bb.addr, owners[i].bb.size);
        break;
      }      
    }
  } 
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  printf("Llame a end_process");
  int pos = curr_proc_pos(process.pid, owners);

  // liberar el espacio del heap
  owners_space[pos].heap_ptr.addr = 0;
  owners_space[pos].heap_ptr.size = 0;
  // eliminar todos los elementos de free_list_heap
  while(owners_space[pos].free_list_heap.head != NULL){
    delete_node(owners_space[pos].free_list_heap, owners_space[pos].heap_ptr.addr);
  }
  // actualizar los valores del stack pointer;
  owners_space[pos].stack_pointer.addr = owners[pos].bb.size;
        owners_space[pos].stack_pointer.size = 0;
  // ownership
  m_unset_owner(owners[pos].bb.addr, owners[pos].bb.size);
  // actualizar el pid en la posición `pos` de la lista owners
  owners[pos].pid = -1;
}
