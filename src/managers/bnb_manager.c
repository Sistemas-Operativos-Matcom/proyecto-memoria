#include "bnb_manager.h"
#include "stdio.h"
#include "../memory.h"

#define MAX_PROCESS_COUNT 20
int *pids_bnb;    // array de los pids
int index_bnb;    // posicion del proceso actual
addr_t *base_bnb; // array de los base
addr_t *heap_bnb;
addr_t *stack_bnb;
addr_t bound_bnb;
addr_t *free_list_bnb;

addr_t pa_bnb(addr_t virtual_adress)
{
  return base_bnb[index_bnb] + virtual_adress;
}

int check_addr_bnb(addr_t virtual_adress)
{
  return virtual_adress >= bound_bnb;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  pids_bnb = (int *)malloc(MAX_PROCESS_COUNT * sizeof(int));
  base_bnb = (addr_t *)malloc(MAX_PROCESS_COUNT * sizeof(addr_t));
  heap_bnb = (addr_t *)malloc(MAX_PROCESS_COUNT * sizeof(addr_t));
  stack_bnb = (addr_t *)malloc(MAX_PROCESS_COUNT * sizeof(addr_t));
  free_list_bnb = (addr_t *)malloc(m_size() * sizeof(addr_t));
  index_bnb = -1;
  bound_bnb = m_size() / MAX_PROCESS_COUNT;
  for (size_t i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    pids_bnb[i] = -1;
    base_bnb[i] = i * bound_bnb;
    stack_bnb[i] = bound_bnb - 1;
    heap_bnb[i] = 0;
  }
  for (size_t i = 0; i < m_size(); i++)
  {
    free_list_bnb[i] = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  if (heap_bnb[index_bnb] + size < stack_bnb[index_bnb])
  {
    out->addr = heap_bnb[index_bnb];
    for (addr_t i = base_bnb[index_bnb] + heap_bnb[index_bnb]; i < base_bnb[index_bnb] + heap_bnb[index_bnb] + size; i++)
    {
      free_list_bnb[i] = 1;
    }
    heap_bnb[index_bnb] +=  size;
    return 0;
  }
  return 1;
  
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  if (ptr.size + ptr.addr > bound_bnb)
  {
    return 1;
  }
  m_unset_owner(pa_bnb(ptr.addr), pa_bnb(ptr.addr) + ptr.size);
  for (size_t i = pa_bnb(ptr.addr); i < pa_bnb(ptr.addr) + ptr.size; i++)
  {
    free_list_bnb[i] = 0;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (stack_bnb[index_bnb] <= heap_bnb[index_bnb])
    return 1;
  m_write(pa_bnb(stack_bnb[index_bnb]), val);
  free_list_bnb[pa_bnb(stack_bnb[index_bnb])] = 1;
  out->addr = stack_bnb[index_bnb];
  stack_bnb[index_bnb]--;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (stack_bnb[index_bnb] >= bound_bnb - 1)
    return 1;
  stack_bnb[index_bnb]++;
  free_list_bnb[pa_bnb(stack_bnb[index_bnb])] = 0;
  *out = m_read(pa_bnb(stack_bnb[index_bnb]));
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if(free_list_bnb[pa_bnb(addr)] && addr < bound_bnb)
  {
    *out = m_read(pa_bnb(addr));
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (free_list_bnb[pa_bnb(addr)] && addr < bound_bnb)
  {
    m_write(pa_bnb(addr), val);
    return 0;
  }
  return 1;
  
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  for (size_t i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    if (pids_bnb[i] == process.pid)
    { 
      index_bnb = i;
      return;
    }
  }
  for (size_t i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    if (pids_bnb[i] == -1)
    {
      pids_bnb[i] = process.pid;
      index_bnb = i;
      m_set_owner(base_bnb[index_bnb], base_bnb[index_bnb] + bound_bnb);
      return;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (size_t i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    if (pids_bnb[i] == process.pid)
    {
      pids_bnb[i] = -1;
      stack_bnb[i] = bound_bnb - 1;
      heap_bnb[i] = 0;
      for (size_t j = base_bnb[i]; j < base_bnb[i] + bound_bnb; j++)
      {
        free_list_bnb[j] = 0;
      }
      break;
    }
  }
}
