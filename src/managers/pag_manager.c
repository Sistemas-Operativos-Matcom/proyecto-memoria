#include "pag_manager.h"

#include "stdio.h"

#define PAGE_SIZE 64
#define STACK_SIZE 64
#define HEAP_SIZE 512

#define PAGES_NUMBER(addr) (addr < PAGE_SIZE) ? 1 : (addr % PAGE_SIZE) > 0 ? (addr / PAGE_SIZE) +1: (addr / PAGE_SIZE);
#define ADDR_PAGE(addr) PAGES_NUMBER(addr);
#define ADDR_SHIFT(addr) addr < PAGE_SIZE ? addr : addr % PAGE_SIZE;

//Nodo de la lista de page frames libres
typedef struct pag_free_node {
  byte pag_frame_id;//id del page frame
  struct pag_free_node *next; //proximo page frame
} pag_free_node_t;

//Lista de page frames libres (header)
pag_free_node_t *pag_free_list_head = NULL;  

//estructura con los datos de la memoria utilizada por un proceso
typedef struct pag_proc_mem_info {
  int pid; //id del proceso
  pag_free_node_t *proccess_pages; //lista de los page frames utilizados por el proceso
  pag_free_node_t *heap_pages;  //lista de los page frames utilizados por el heap
  uint heap_pos;//posición en el heap
  uint heap_size;//tamaño del heap
  pag_free_node_t *stack_pages;  //lista de los page frames utilizados por el stack
  uint stack_pos;//posicion en el stack
  uint stack_size;//tamaño del stack
} pag_proc_mem_info_t;

//structura de la lista de procesos
typedef struct pag_proc_mem_info_node {
  pag_proc_mem_info_t proc_mem_info;//información del proceso
  struct pag_proc_mem_info_node *next;//proximo proceso
} pag_proc_mem_info_node_t;


pag_proc_mem_info_node_t *pag_proc_mem_info_list_head = NULL;  //lista de procesos (header)
pag_proc_mem_info_node_t *pag_current_proccess = NULL; //proceso que se esta ejecutando



void show_list(pag_free_node_t *list){
  pag_free_node_t *temp_mem = list;

  while(temp_mem!=NULL){
    printf(" %i ", temp_mem->pag_frame_id);
    temp_mem = temp_mem->next;
  }
  printf("\n");
  
}

//Reservar los pages frames libres
pag_free_node_t *pag_alloc(size_t size){
  pag_free_node_t *temp_mem = pag_free_list_head;//temporal apuntando a la lista
  pag_free_node_t *result = pag_free_list_head;//resultado de la asignación, apunta al primer elemento de la lista de pages frames libres
  int cnodes = PAGES_NUMBER(size);//cantidad de pages frames necesarios para la asignación

  //recorro los primeros cnodes de mi lista de pages frames libres
  for(;cnodes>1;cnodes--)
    if(temp_mem->next!=NULL)  
      temp_mem=temp_mem->next;
    else
      return NULL;//si no alcanzan, retorno NULL
  //separo la lista en dos listas, los primeros cnodes para devolver y la lista de pages frames libres que quedan
  pag_free_list_head = temp_mem->next;
  temp_mem->next = NULL;
  temp_mem = result;
  //hacer propietario al proceso de la memoria reservada
  while (temp_mem != NULL){ 
    m_set_owner(temp_mem->pag_frame_id*PAGE_SIZE,temp_mem->pag_frame_id*PAGE_SIZE+PAGE_SIZE-1);
    temp_mem=temp_mem->next;
  }
  
  return result;
}

//Liberar los pages frames
void pag_free(pag_free_node_t *pag_list){
  pag_free_node_t *temp_mem = pag_list;
  pag_free_node_t *previus = NULL;

    
  //quitar propietario al proceso de la memoria reservada
  while (temp_mem != NULL){ 
    m_unset_owner(temp_mem->pag_frame_id*PAGE_SIZE,temp_mem->pag_frame_id*PAGE_SIZE+PAGE_SIZE-1);
    if(temp_mem->next == NULL)
      previus = temp_mem;
    temp_mem=temp_mem->next;
  }
  //unificar la lista de pages frame pasada con la lista de pages frames libres
  previus->next = pag_free_list_head;
  pag_free_list_head = pag_list;
    
}
//escribir en la memoria
void pag_write(pag_free_node_t *pag_list, addr_t addr, byte val){
  int pag_frame_pos = ADDR_PAGE(addr);//posicion del page frame
  pag_free_node_t *temp_mem = pag_list;
  addr_t addr_shift = ADDR_SHIFT(addr);//desplazamiento de la dirección

   //recorro la lista hasta el page frame donde esta la dirección
  for(int i=1;i < pag_frame_pos; i++)
    temp_mem = temp_mem->next;
  //escribir en el page frame
  m_write(temp_mem->pag_frame_id*PAGE_SIZE+addr_shift,val);
}

//leer de la memoria
byte pag_read(pag_free_node_t *pag_list, addr_t addr){
  int pag_frame_pos = ADDR_PAGE(addr);//posicion del page frame
  pag_free_node_t *temp_mem = pag_list;
  addr_t addr_shift = ADDR_SHIFT(addr);//desplazamiento de la dirección
  
  //recorro la lista hasta el page frame donde esta la dirección
  for(int i=1;i < pag_frame_pos; i++)
    temp_mem = temp_mem->next;
  return m_read(temp_mem->pag_frame_id*PAGE_SIZE+addr_shift);//leer desde el page frame
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  pag_proc_mem_info_node_t *temp;
  pag_free_node_t *temp_mem;
  //si queda de un caso anterior, libero la lista de page frames libres
  while (pag_free_list_head != NULL) {
      temp_mem = pag_free_list_head;
      pag_free_list_head = pag_free_list_head->next;
      free(temp_mem);
  }
  
  pag_free_list_head=NULL;

  //si queda de un caso anterior, libero la lista de procesos
  while (pag_proc_mem_info_list_head != NULL) {
      temp = pag_proc_mem_info_list_head;
      pag_proc_mem_info_list_head = pag_proc_mem_info_list_head->next;
      free(temp);
  }
  
  pag_current_proccess = NULL;//no hay proceso actual

  //inicializo mi lista de pages frames libres
  for(int i = m_size()/PAGE_SIZE; i > 0 ;i--){
      temp_mem = (pag_free_node_t *) malloc(sizeof(pag_free_node_t));
      temp_mem->pag_frame_id = i-1;
      temp_mem->next = pag_free_list_head;
      pag_free_list_head = temp_mem;
  }
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  
  //si la direccion esta en el espacio de direcciones del heap
  if (pag_current_proccess->proc_mem_info.heap_pos + size < pag_current_proccess->proc_mem_info.heap_size){
    out->addr = pag_current_proccess->proc_mem_info.heap_pos;//reservo
    out->size = size;
    pag_current_proccess->proc_mem_info.heap_pos += size + 1; //incremento posicion en el heap 
    return 0;
  }
  else
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  //si la direccion esta en el espacio de direcciones del heap
  if (ptr.addr <=pag_current_proccess->proc_mem_info.heap_pos){
    if (ptr.addr == pag_current_proccess->proc_mem_info.heap_pos - ptr.size - 1)
      pag_current_proccess->proc_mem_info.heap_pos -= ptr.size;
    return 0;
  }
  else
    return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //si hay espacio libre en el stack
  if (pag_current_proccess->proc_mem_info.stack_pos - 1 > 0){
    pag_current_proccess->proc_mem_info.stack_pos -= 1;//decremento la posicion en el stack
    pag_write(pag_current_proccess->proc_mem_info.stack_pages, pag_current_proccess->proc_mem_info.stack_pos, val);//escribo el valor
    out->addr =  pag_current_proccess->proc_mem_info.stack_pos;//retorno direccion
    out->size = 1;
    return 0;
  }
  else
    return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  //si no me salgo del espacio del stack
  if (pag_current_proccess->proc_mem_info.stack_pos + 1 <= pag_current_proccess->proc_mem_info.stack_size){
    *out = pag_read(pag_current_proccess->proc_mem_info.stack_pages,pag_current_proccess->proc_mem_info.stack_pos);//leo el valor
    pag_current_proccess->proc_mem_info.stack_pos += 1;//incremento la posicion en el stack
    return 0;
  }
  else
    return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //si la direccion esta en el espacio del heap
  if (addr <= pag_current_proccess->proc_mem_info.heap_size){
    *out = pag_read(pag_current_proccess->proc_mem_info.heap_pages,addr);//leo el valor
    return 0;
  }
  else
    return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  //si la direccion esta en el espacio del heap
  if (addr <= pag_current_proccess->proc_mem_info.heap_size){
    pag_write(pag_current_proccess->proc_mem_info.heap_pages,addr,val);//escribo el valor
    return 0;
  }
  else
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  pag_proc_mem_info_node_t *temp = pag_proc_mem_info_list_head;

  //primero busco si ya el proceso existe
  while (temp != NULL) {
    if (temp->proc_mem_info.pid == process.pid) {//si ya existe
      pag_current_proccess = temp;//lo pongo como proceso actual
      return;    
    }
    temp = temp->next;
  }
  //si no existe el proceso, tengo que crear un nuevo elemento de la lista de procesos
  pag_current_proccess = (pag_proc_mem_info_node_t *) malloc(sizeof(pag_proc_mem_info_node_t));//reservo memoria para info proceso
  pag_current_proccess->proc_mem_info.pid = process.pid;// id del proceso
  
  pag_current_proccess->proc_mem_info.proccess_pages = pag_alloc(round_proccess_size(process.program->size));//reservo espacio para el codigo del proceso
  //Comprobar que se reservó bien la memoria para el proceso
  if (pag_current_proccess->proc_mem_info.proccess_pages ==NULL){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(pag_current_proccess);
    pag_current_proccess = NULL;
    return;
  }
  pag_current_proccess->proc_mem_info.heap_pages = pag_alloc(HEAP_SIZE);//reservo espacio para el heap
  //Comprobar que se reservó bien la memoria para el heap
  if (pag_current_proccess->proc_mem_info.heap_pages ==NULL){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(pag_current_proccess);
    pag_current_proccess = NULL;
    return;
  }
  pag_current_proccess->proc_mem_info.heap_size = HEAP_SIZE;
  pag_current_proccess->proc_mem_info.heap_pos = 0;
  pag_current_proccess->proc_mem_info.stack_pages = pag_alloc(STACK_SIZE);//reservo espacio para el stack
  //Comprobar que se reservó bien la memoria para el stack
  if (pag_current_proccess->proc_mem_info.stack_pages == NULL){
    printf("Error al reservar memoria para proceso %d. No hay suficiente memoria\n",process.pid);
    free(pag_current_proccess);
    pag_current_proccess = NULL;
    return;
  }
  pag_current_proccess->proc_mem_info.stack_size = STACK_SIZE;
  pag_current_proccess->proc_mem_info.stack_pos = STACK_SIZE;
  //adiciono el proceso a mi lista de procesos
  pag_current_proccess->next = pag_proc_mem_info_list_head;
  pag_proc_mem_info_list_head = pag_current_proccess;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  pag_proc_mem_info_node_t *temp = pag_proc_mem_info_list_head;
  pag_proc_mem_info_node_t *previus = NULL;

    // busco el proceso actual en mi lista de procesos
    while (temp != NULL) {
      if (temp->proc_mem_info.pid == process.pid) {
        if (previus == NULL)
          pag_proc_mem_info_list_head = temp->next;
        else 
          previus->next = temp->next;
      
      pag_free(temp->proc_mem_info.proccess_pages);//libero espacio para el código del proceso
      pag_free(temp->proc_mem_info.heap_pages);//libero espacio para el heap
      pag_free(temp->proc_mem_info.stack_pages);//libero espacio para el stack
      free(temp);//libero memoria
      return;    
    }
    previus = temp;
    temp = temp->next;
  }
}
