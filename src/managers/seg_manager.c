#include "seg_manager.h"

#include "stdio.h"
#define OUT_OF_LIMITS 20000ul
#define NO_FREE_MEMORY OUT_OF_LIMITS + 1ul
#define UL unsigned long

/*
addr_t vmem_size = 256ul;
UL curr_section;
int free_mem[100][256];

addr_t search_free(size_t size){

  unsigned int counter = 0u;

  for(addr_t c = 0ul ; c < vmem_size;c++){

    if(free_mem[curr_section][c] == FREE){
      counter++;
    }else{
      counter = 0u;
    }
    if(counter == size){
      for(unsigned long d = c - size;d < c;d++){
        free_mem[curr_section][d] = USED;
      }
      return c - size + 1ul;
    }
  }
  return NO_FREE_MEMORY;

}
*/

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) {

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {/*
  addr_t v_address = search_free(size);
  if(v_address != NO_FREE_MEMORY){
    
   (*out).addr = v_address;
   (*out).size = size;
   
   return 0;

  }

 return 1;*/
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {/*
  UL section = search_free_section(process.pid);

    curr_section = section;
    m_set_owner(curr_section * vmem_size ,curr_section * vmem_size + 256ul);//create heap
    m_set_owner(m_size - (curr_section + 1) * vmem_size - 1,m_size - curr_section * vmem_size -1 );//create stack

*/
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
