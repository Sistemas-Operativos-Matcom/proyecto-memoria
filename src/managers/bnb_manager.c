#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"

#define MAX_PROCS_COUNT 20
int *bnb_pids;
addr_t *bnb_base;
addr_t *bnb_heap;
addr_t *bnb_stack;
addr_t *bnb_free_list;
int bnb_index_curr_pid;
addr_t bnb_bound;

addr_t bnb_pa(addr_t va)
{
  return bnb_base[bnb_index_curr_pid] + va;
}
int bnb_check_addr(addr_t va)
{
  return va >= bnb_bound;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  bnb_pids = (int *)malloc(MAX_PROCS_COUNT * sizeof(int));
  bnb_base = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_heap = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_stack = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_free_list = (addr_t *)malloc(m_size() * sizeof(addr_t));

  bnb_index_curr_pid = -1;
  bnb_bound = m_size() / MAX_PROCS_COUNT;
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    bnb_pids[i] = -1;
    bnb_base[i] = i * bnb_bound;
    bnb_stack[i] = bnb_bound - 1;
    bnb_heap[i] = 0;
  }
  for (size_t i = 0; i < m_size(); i++)
  {
    bnb_free_list[i] = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  if (bnb_heap[bnb_index_curr_pid] + size < bnb_stack[bnb_index_curr_pid])
  {
    out->addr = bnb_heap[bnb_index_curr_pid];
    out->size = size;

    for (addr_t i = bnb_base[bnb_index_curr_pid] + bnb_heap[bnb_index_curr_pid]; i < bnb_base[bnb_index_curr_pid] + bnb_heap[bnb_index_curr_pid] + size; i++)
    {
      bnb_free_list[i] = 1;
    }
    bnb_heap[bnb_index_curr_pid] += size;
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  if (ptr.size + ptr.addr > bnb_bound)
  {
    return 1;
  }
  m_unset_owner(bnb_pa(ptr.addr), bnb_pa(ptr.addr) + ptr.size);
  for (size_t i = bnb_pa(ptr.addr); i < bnb_pa(ptr.addr) + ptr.size; i++)
  {
    bnb_free_list[i] = 0;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (bnb_stack[bnb_index_curr_pid] <= bnb_heap[bnb_index_curr_pid])
    return 1;
  m_write(bnb_pa(bnb_stack[bnb_index_curr_pid]), val);
  bnb_free_list[bnb_pa(bnb_stack[bnb_index_curr_pid])] = 1;
  out->addr = bnb_stack[bnb_index_curr_pid];
  out->size = 1;
  bnb_stack[bnb_index_curr_pid]--;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (bnb_stack[bnb_index_curr_pid] >= bnb_bound - 1)
    return 1;
  bnb_stack[bnb_index_curr_pid]++;
  bnb_free_list[bnb_pa(bnb_stack[bnb_index_curr_pid])] = 0;
  *out = m_read(bnb_pa(bnb_stack[bnb_index_curr_pid]));
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if (addr < bnb_bound && bnb_free_list[bnb_pa(addr)])
  {
    *out = m_read(bnb_pa(addr));
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (addr < bnb_bound && bnb_free_list[bnb_pa(addr)])
  {
    m_write(bnb_pa(addr), val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (bnb_pids[i] == process.pid)
    {
      bnb_index_curr_pid = i;
      set_curr_owner(process.pid);
      return;
    }
  }
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (bnb_pids[i] == -1)
    {
      bnb_index_curr_pid = i;
      bnb_pids[i] = process.pid;
      m_set_owner(bnb_base[bnb_index_curr_pid], bnb_base[bnb_index_curr_pid] + bnb_bound - 1);
      return;
    }
  }
  return;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (process.pid == bnb_pids[i])
    {
      bnb_pids[i] = -1;
      bnb_stack[i] = bnb_bound - 1;
      bnb_heap[i] = 0;
      for (size_t j = bnb_base[i]; j < bnb_base[i] + bnb_bound; j++)
      {
        bnb_free_list[j] = 0;
      }
      break;
    }
  }
}
