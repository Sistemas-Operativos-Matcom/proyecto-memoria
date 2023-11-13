#include "bnb_manager.h"
#include "stdio.h"
#include "freelist.h"

#define STACK_SIZE 100
typedef struct{

  size_t code_size;
  
  size_t stack_pointer; 
  
  int owner;

  struct Freelist* freelist;

} BnbBlock;

int bnb_cantBlock;

int bnb_BlockBound;


BnbBlock* bnbBlocks = NULL; // Array de bloques

int bnb_current_process_index;


#define current_frame_base bnb_current_process_index*bnb_BlockBound;



// Esta función  inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  
  // Lista de BnB frames en memoria fisica
  bnb_BlockBound = 1024;
  
  bnb_cantBlock = m_size()/bnb_BlockBound;

  free(bnbBlocks);
  bnbBlocks = malloc(bnb_cantBlock*sizeof(BnbBlock));

  for(int i = 0; i<bnb_cantBlock; i++){
    
    bnbBlocks[i].owner = NO_ONWER;
  }

}

/* Reserva un espacio en el heap y retorna un puntero que apunta a donde comienza ese espacio.*/

int m_bnb_malloc(size_t size, ptr_t *out) {
  
  BnbBlock* block = &(bnbBlocks[bnb_current_process_index]);
  
  out->size = size;
  
  int rc = freelist_alloc_space(block->freelist, size, &(out->addr));
  
  return rc;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  BnbBlock* block = &(bnbBlocks[bnb_current_process_index]);
  freelist_free_space(block->freelist, ptr.addr, ptr.size);
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
 
  BnbBlock* block = &(bnbBlocks[bnb_current_process_index]);
  size_t frame_base = current_frame_base;
  
  // Disminuye el SP  
  block->stack_pointer -= 1;
  
  //comprobar si hubo Stack Overflow
  if(block->stack_pointer <= block->code_size-1){
    return -1;
  }

  // Inserta el elemento
  m_write(frame_base + block->stack_pointer, val);
  out->addr = block->stack_pointer;
  out->size = 1;

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  
  // Leer lo ultimo del stack
  BnbBlock* block = &(bnbBlocks[bnb_current_process_index]);
  size_t frame_base = current_frame_base;
  *out = m_read(frame_base + block->stack_pointer);

  // Aumentar el SP y comprobar si hubo Stack Underflow
  block->stack_pointer += 1;
  if(block->stack_pointer  > block->code_size+STACK_SIZE){
    return -1;
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if(addr > (addr_t)bnb_BlockBound-1){
    return -1;
  }
  size_t aux= current_frame_base;
  *out = m_read(aux + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  BnbBlock* block = &(bnbBlocks[bnb_current_process_index]);
  
  if(addr > (addr_t)bnb_BlockBound-1 || addr <= block->code_size-1){
    return -1;
  }
  size_t aux = current_frame_base;
  m_write(aux+addr, val);
  return 0;
}

// Realiza un cambio de contexto  
void m_bnb_on_ctx_switch(process_t process) {
  set_curr_owner(process.pid);

  // Buscar el trocito de memoria correspondiente al proceso
  int indexFirstFree = -1;
  
  for(int i = 0; i<bnb_cantBlock; i++){
    if(process.pid == bnbBlocks[i].owner){
      bnb_current_process_index = i;
      
      return;
    }
    if(indexFirstFree == -1 && bnbBlocks[i].owner == NO_ONWER){
      
      indexFirstFree = i;
    }
  }

  // Si el proceso no tiene memoria, darle (de ser posible)
  if (indexFirstFree == -1){
    exit(-1);
  }
  
  bnbBlocks[indexFirstFree].owner = process.pid;
  
  bnbBlocks[indexFirstFree].code_size = process.program->size;
  

  bnbBlocks[indexFirstFree].stack_pointer = process.program->size+STACK_SIZE;
  
  bnbBlocks[indexFirstFree].freelist = freelist_init(process.program->size+STACK_SIZE, bnb_BlockBound-(process.program->size+STACK_SIZE));
  
  m_set_owner(indexFirstFree*bnb_BlockBound, indexFirstFree*bnb_BlockBound+bnb_BlockBound-1);
  
  bnb_current_process_index = indexFirstFree;
}

// Termina la ejecucion de un proceso
void m_bnb_on_end_process(process_t process) {
  for(int i = 0; i<bnb_cantBlock; i++){
    if(process.pid == bnbBlocks[i].owner){
      
      bnbBlocks[i].owner = NO_ONWER;
      
      m_unset_owner( i*bnb_BlockBound, i*bnb_BlockBound + bnb_BlockBound-1);
      
      freelist_deinit(bnbBlocks[i].freelist);
      return;
    }
  }
}
