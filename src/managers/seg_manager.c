#include "seg_manager.h"

#include "stdio.h"
#define OUT_OF_LIMITS 20000ul
#define NO_FREE_MEMORY OUT_OF_LIMITS + 1ul
#define UL unsigned long


int heap_size = 256;
int stack_size = 256;
int curr_proc = -1;
section_t heaps[50];
section_t stacks[50];
int process_pos[50];

addr_t seg_address_translator(addr_t v_address){
  if(v_address < heap_size){
   return (curr_proc * heap_size) + v_address;

  }else{
    return OUT_OF_LIMITS;
  }
 

}

addr_t seg_search_free(size_t size){

  unsigned int counter = 0u;
  for(addr_t c = 0ul ; c < heap_size;c++){
    if(heaps[curr_proc].heap[c] == FREE){
      counter++;
    }else{
      counter = 0u;
    }
    if(counter == size){
      for(unsigned long d = c - size;d < c;d++){
        heaps[curr_proc].heap[d] = USED;
      }
      return c - size + 1ul;
    }
  }
  return NO_FREE_MEMORY;
}


// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv){

 for(int c = 0;c<50;c++)process_pos[c] = -1;

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {
  
  addr_t temp_address = seg_search_free(size);
  if(temp_address != OUT_OF_LIMITS){
    (*out).size = size;
    (*out).addr = temp_address;
    return 0;
  }

  return 1;

}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {
  
  for(int c = 0;c<ptr.size;c++){
    heaps[curr_proc].heap[ptr.addr + c] = FREE;
  }

}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {
 (*out).size = 1ul;
 if(stacks[curr_proc].stack_pointer == 0)return 1;

  stacks[curr_proc].stack_pointer--;
  (*out).addr = stacks[curr_proc].stack_pointer;
  m_write( stacks[curr_proc].real_base + stacks[curr_proc].stack_pointer,val);
  return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {

  if(stacks[curr_proc].stack_pointer >= 256)return 1;
  *out = m_read( stacks[curr_proc].real_base + stacks[curr_proc].stack_pointer );

  stacks[curr_proc].stack_pointer++;  

  return 0;

}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {

  *out = m_read(seg_address_translator(addr));

  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {

  m_write(seg_address_translator(addr),val);

  return 0;
}

int search_free_pos(int pid,int *new){
  int result = 0;
  int found_free = 0;
  for(int c = 0 ;c < 50;c++){
   
   if(process_pos[c] == -1 && found_free == 0){
      result = c;
      found_free = 1;
     }

   if(process_pos[c] == pid){
      *new = 1; 
      return c;

     }

  }
  process_pos[result] = pid;
  
  return result;

}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {
  int new = 0;
  curr_proc = search_free_pos(process.pid,&new);

  if(new == 0){
    short heap[50];
  
    for(int c = 0;c < 512;c++){

      heap[c] = FREE;

    }
    heaps[curr_proc] = (section_t){curr_proc * heap_size,heap_size ,&heap,-1};
    m_set_owner(curr_proc * heap_size,curr_proc * heap_size + heap_size);
    stacks[curr_proc] = (section_t){m_size() - curr_proc * stack_size - stack_size - 1,stack_size ,NULL,255};
    stacks[curr_proc].stack_pointer = 256;
    m_set_owner(m_size() - (curr_proc * stack_size + stack_size) ,m_size() - curr_proc * stack_size - 1);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  int n;

  int posic = search_free_pos(process.pid,&n);

  process_pos[posic] = -1;

  m_unset_owner(posic * heap_size,posic * heap_size + heap_size);
  m_unset_owner(m_size() - (posic * stack_size + stack_size) ,m_size() - posic * stack_size - 1);
 

}
