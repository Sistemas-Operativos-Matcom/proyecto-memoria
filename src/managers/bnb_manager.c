#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"

int MAX_PROCS_COUNT = 20;
int *bnb_pids;
addr_t *bnb_base;
addr_t *bnb_heap;
addr_t *bnb_stack;
int *bnb_free_list;
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

  bnb_index_curr_pid = -1;
  bnb_bound = 1024;
  MAX_PROCS_COUNT = m_size() / bnb_bound;
  // bnb_bound = m_size() / MAX_PROCS_COUNT;

  bnb_pids = (int *)malloc(MAX_PROCS_COUNT * sizeof(int));
  bnb_base = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_heap = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_stack = (addr_t *)malloc(MAX_PROCS_COUNT * sizeof(addr_t));
  bnb_free_list = (int *)malloc(m_size() * sizeof(int));

  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    bnb_pids[i] = -1;
    bnb_base[i] = i * bnb_bound;
    bnb_stack[i] = bnb_bound;
    bnb_heap[i] = 0;
  }

  for (size_t i = 0; i < m_size(); i++)
  {
    bnb_free_list[i] = -1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  for (addr_t i = bnb_pa(0); i < bnb_stack[bnb_index_curr_pid] + bnb_pa(0); i++)
  {
    if (bnb_free_list[i] == bnb_index_curr_pid)
    {
      bnb_heap[bnb_index_curr_pid] = i + 1;
    }
    else
    {
      bnb_heap[bnb_index_curr_pid] = 0;
    }
  }

  if (bnb_heap[bnb_index_curr_pid] + size <= bnb_stack[bnb_index_curr_pid])
  {
    out->addr = bnb_heap[bnb_index_curr_pid];
    out->size = size;

    for (addr_t i = bnb_pa(out->addr); i < size + bnb_pa(out->addr); i++)
    {
      bnb_free_list[i] = bnb_pids[bnb_index_curr_pid];
    }
    
    bnb_heap[bnb_index_curr_pid] += size;
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{

  if (((int)ptr.addr < 0 || ptr.addr > bnb_bound) || ((int)bnb_free_list[bnb_pa(ptr.addr)] < 0))
  {
    return 1;
  }
  if (ptr.size > 0 || ptr.size < bnb_bound)
  {
    for (addr_t i = ptr.addr; i < ptr.size + ptr.addr; i++)
    {
      bnb_free_list[i] = -1;
    }
  }
  for (addr_t i = bnb_pa(0); i < bnb_stack[bnb_index_curr_pid] + bnb_pa(0); i++)
  {
    if ((int)bnb_free_list[i] == bnb_index_curr_pid)
    {
      bnb_heap[bnb_index_curr_pid] = i + 1;
    }
    else
    {
      bnb_heap[bnb_index_curr_pid] = 0;
    }
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if ((int)bnb_heap[bnb_index_curr_pid] < 0 || bnb_stack[bnb_index_curr_pid] - 1 > bnb_bound)
  {
    return 1;
  }
  bnb_stack[bnb_index_curr_pid]--;
  m_write(bnb_pa(bnb_stack[bnb_index_curr_pid]), val);
  out->addr = bnb_stack[bnb_index_curr_pid];
  out->size = 1;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if ((int)bnb_stack[bnb_index_curr_pid] + 1 < 0 || bnb_stack[bnb_index_curr_pid] + 1 > bnb_bound)
  {
    return 1;
  }
  *out = m_read(bnb_pa(bnb_stack[bnb_index_curr_pid]));
  bnb_stack[bnb_index_curr_pid]++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if (addr < bnb_bound && bnb_free_list[bnb_pa(addr)] >= 0)
  {
    *out = m_read(bnb_pa(addr));
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  printf("freelist %d\n", bnb_free_list[bnb_pa(addr)]);
  if (addr < bnb_bound && bnb_free_list[bnb_pa(addr)] >= 0)
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
      set_curr_owner(process.pid);
      m_set_owner(bnb_pa(0), bnb_pa(bnb_bound - 1));
      return;
    }
  }
  set_curr_owner(bnb_pids[bnb_index_curr_pid]);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (process.pid == bnb_pids[i])
    {
      bnb_pids[i] = -1;
      bnb_stack[i] = bnb_bound;
      bnb_heap[i] = 0;
      m_unset_owner(bnb_pa(0), bnb_pa(bnb_bound - 1));
      for (size_t j = bnb_base[i]; j < bnb_base[i] + bnb_bound; j++)
      {
        bnb_free_list[j] = -1;
      }
      break;
    }
  }
}