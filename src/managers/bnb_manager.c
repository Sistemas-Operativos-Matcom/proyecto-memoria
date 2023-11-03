#include "bnb_manager.h"

#include "stdio.h"


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) 
{
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) 
{
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) 
{
  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) 
{
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) 
{
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) 
{
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) 
{
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) 
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) 
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
