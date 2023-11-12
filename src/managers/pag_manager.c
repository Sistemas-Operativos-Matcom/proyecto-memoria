#include "pag_manager.h"
#include "../memory.h"

#include "bnb_manager.h"
#include "stdio.h"

#define MAX_PROCS_COUNT 20
int *pag_pids;
addr_t *pag_base;
addr_t *pag_free_list;
int pag_index_curr_pid;
addr_t pag_bound;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  pag_pids = (int *)malloc(MAX_PROCS_COUNT * sizeof(int));
  pag_base = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  pag_free_list = (addr_t *)malloc(m_size() * sizeof(addr_t));

  pag_index_curr_pid = -1;
  pag_bound = m_size() / MAX_PROCS_COUNT;
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    pag_pids[i] = -1;
    pag_base[i] = i * pag_bound;
  }
  for (size_t i = 0; i < m_size(); i++)
  {
    pag_free_list[i] = 0;
  }
  m_bnb_init(argc, argv);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_malloc(size, out);
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_free(ptr);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_push(val, out);
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_pop(out);
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_load(addr, out);
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  return m_bnb_store(addr, val);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  m_bnb_on_ctx_switch(process);
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  // fprintf(stderr, "Not Implemented\n");
  // exit(1);
  m_bnb_on_end_process(process);
}
