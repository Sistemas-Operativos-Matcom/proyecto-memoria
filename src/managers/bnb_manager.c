#include "bnb_manager.h"

#include "stdio.h"
#define bound 512


typedef struct Proceso {
  process_t process;
  addr_t base;
  addr_t heap_bottom;
  addr_t stack_top;
  
  struct Proceso_t* prev;
  struct Proceso_t* next;
}Proceso_t;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  fprintf(stderr, "Not Implemented\n");
  //initialize list of proccess
  byte x = bound;


  //inicializar memoria virtual
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");

  //buscar primer espacio libre que space.size > size, fraccionarlo y asignar al puntero un si puntero, si todo salio bien
  
  
  //out->addr =  
  out->size = size;

  
  //return ptr
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");

  //si el addr pertenece al heap y está ocupado, liberar e intentar unir con pedazos adyacentes.
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");

  // si el stack_top + 1 <  heap bottom escribir en out.addrs el valor val y stack_top++
  exit(1);
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  //si el stack_top > 0 devolver el ultimo valor y stack_top--;
  exit(1);
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  fprintf(stderr, "Not Implemented\n");
  
  //real addr = base + addr;
  addr_t inMemory = addr;
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  //cambiar el current process y si el process pid no existe en la lista de procesos, añadirlo

  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  //process.program->size
  fprintf(stderr, "Not Implemented\n");

  // liberar el heap y el proceso. poner en 0 el slot del proceso
  exit(1);
}
