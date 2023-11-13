#include "pag_manager.h"
#include "../memory.h"

#include "bnb_manager.h"
#include "stdio.h"

#define MAX_PROCS_COUNT 20
int **pag_page_table;
int *pag_pids;
int *pag_free_list;
addr_t *pag_heap;
addr_t *pag_stack;
addr_t *pag_virtual_m;
int pag_index_curr_pid;
addr_t pag_offset;
addr_t pag_page_size;
int pag_pages_count;

addr_t pag_get_base_of_page(int physical_page)
{
  return physical_page * pag_page_size;
}
addr_t pag_get_offset(addr_t addr)
{
  return addr & ((1 << pag_offset) - 1);
}
addr_t pag_get_vpn(addr_t addr)
{
  return addr >> pag_offset;
}
addr_t pag_get_pa(addr_t addr)
{
  int pp = pag_page_table[pag_index_curr_pid][pag_get_vpn(addr)];
  return pag_get_base_of_page(pp) + pag_get_offset(addr);
}
int pag_free_space(addr_t size, addr_t *l)
{
  int free_pages = 0;
  for (int i = 0; i < pag_pages_count; i++)
  {
    free_pages += pag_free_list[i];
  }

  addr_t count = 0;
  *l = __INT_MAX__;
  int aux = free_pages;

  for (addr_t vpn = 0; vpn < pag_get_vpn(pag_stack[pag_index_curr_pid]); vpn++)
  {
    if (pag_page_table[pag_index_curr_pid][vpn] >= 0)
    {
      int phisical_page = pag_page_table[pag_index_curr_pid][vpn];
      for (addr_t j = phisical_page * pag_page_size; j < phisical_page * pag_page_size + pag_page_size; j++)
      {
        count++;
        *l = (*l < (vpn * pag_page_size + j)) ? *l : vpn * pag_page_size + j;
        if (pag_virtual_m[j])
        {
          count = 0;
          *l = __INT_MAX__;
          aux = free_pages;
        }
        if (count >= size)
          return 0;
      }
    }
    else if (aux > 0)
    {
      aux--;
      count += pag_page_size;
      *l = (*l < vpn * pag_page_size) ? *l : vpn * pag_page_size;

      if (count >= size)
        return 0;
    }
  }
  return -1;
}
int pag_free_page()
{
  for (int i = 0; i < pag_pages_count; i++)
  {
    if (pag_free_list[i])
    {
      return i;
    }
  }
  return -1;
}
int pag_res_free_m(addr_t virtual_addr, int x, int amount)
{
  for (addr_t i = virtual_addr; i < virtual_addr + x; i++)
  {
    if ((amount && pag_virtual_m[pag_get_pa(i)]) || (!amount && !pag_virtual_m[pag_get_pa(i)]))
    {
      return 1;
    }
    pag_virtual_m[pag_get_pa(i)] = amount;
    if ((pag_get_vpn(i) != pag_get_vpn(virtual_addr)) && (pag_get_pa(i) + 1 != pag_get_pa(i + 1)))
    {
      if (amount)
        m_set_owner(pag_get_pa(virtual_addr), pag_get_pa(i - 1));
      else
        m_unset_owner(pag_get_pa(virtual_addr), pag_get_pa(i - 1));
      virtual_addr = i;
    }
  }

  if (amount)
    m_set_owner(pag_get_pa(virtual_addr), pag_get_pa(virtual_addr + x - 1));
  else
    m_unset_owner(pag_get_pa(virtual_addr), pag_get_pa(virtual_addr + x - 1));

  pag_heap[pag_index_curr_pid] = -1;
  for (addr_t i = 0; i < pag_get_vpn(pag_stack[pag_index_curr_pid]); i++)
  {
    if (pag_page_table[pag_index_curr_pid][i] >= 0)
    {
      pag_heap[pag_index_curr_pid] = i;
    }
  }
  pag_heap[pag_index_curr_pid]++;
  for (addr_t vpn = 0; vpn < pag_heap[pag_index_curr_pid] + 1; vpn++)
  {
    if (!(pag_page_table[pag_index_curr_pid][vpn] >= 0))
      continue;
    int flag = 1;
    int phisical_page = pag_page_table[pag_index_curr_pid][vpn];
    for (addr_t i = pag_get_base_of_page(phisical_page); i < pag_get_base_of_page(phisical_page) + pag_page_size; i++)
    {
      if (pag_virtual_m[i])
      {
        flag = 0;
        break;
      }
    }

    if (flag)
    {
      pag_free_list[phisical_page] = 1;
      pag_page_table[pag_index_curr_pid][vpn] = -1;
    }
  }
  return 0;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  pag_pids = (int *)malloc(sizeof(int) * MAX_PROCS_COUNT);
  pag_stack = (addr_t *)malloc(sizeof(int) * MAX_PROCS_COUNT);
  pag_heap = (addr_t *)malloc(sizeof(addr_t) * MAX_PROCS_COUNT);
  pag_page_table = (int **)malloc(sizeof(int) * MAX_PROCS_COUNT);
  pag_page_size = (m_size() < (addr_t)(1 << pag_offset)) ? m_size() : (size_t)(1 << pag_offset);
  pag_pages_count = m_size() / pag_page_size;
  pag_free_list = (int *)malloc(sizeof(int) * pag_pages_count);

  pag_index_curr_pid = -1;
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    pag_pids[i] = -1;
    pag_stack[i] = pag_page_size * pag_pages_count - 1;
    pag_heap[i] = 0;
    pag_page_table[i] = (int *)malloc(sizeof(int) * pag_pages_count);

    for (int vpn = 0; vpn < pag_pages_count; vpn++)
    {
      pag_page_table[i][vpn] = -1;
    }
  }
  for (int i = 0; i < pag_pages_count; i++)
  {
    pag_free_list[i] = 1;
  }
  pag_virtual_m = (addr_t *)malloc(sizeof(addr_t) * m_size());
  for (addr_t i = 0; i < m_size(); i++)
  {
    pag_virtual_m[i] = 0;
  }
  pag_offset = 6;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  pag_heap[pag_index_curr_pid] = -1;
  for (addr_t i = 0; i < pag_get_vpn(pag_stack[pag_index_curr_pid]); i++)
  {
    if (pag_page_table[pag_index_curr_pid][i] >= 0)
    {
      pag_heap[pag_index_curr_pid] = i;
    }
  }
  pag_heap[pag_index_curr_pid]++;

  addr_t li;
  if (pag_free_space(size, &li) < 0)
  {
    li = -1;
  }
  else
  {
    size_t vpn = pag_get_vpn(li);
    while (vpn <= pag_get_vpn(li + size - 1))
    {
      if (!(pag_page_table[pag_index_curr_pid][vpn] >= 0))
      {
        int free_page = pag_free_page();
        pag_page_table[pag_index_curr_pid][vpn] = free_page;
        pag_free_list[free_page] = 0;
      }
      vpn++;
    }
    pag_res_free_m(li, size, 1);
  }
  if ((int)li < 0)
  {
    return 1;
  }
  out->addr = li;
  out->size = size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  addr_t vpn = pag_get_vpn(ptr.addr);
  while (vpn <= pag_get_vpn(ptr.addr + ptr.size - 1))
  {
    if (!(pag_page_table[pag_index_curr_pid][vpn] >= 0))
    {
      return 1;
    }
    vpn++;
  }
  int x = pag_res_free_m(ptr.addr, ptr.size, 0);
  pag_heap[pag_index_curr_pid] = -1;
  for (addr_t i = 0; i < pag_get_vpn(pag_stack[pag_index_curr_pid]); i++)
  {
    if (pag_page_table[pag_index_curr_pid][i] >= 0)
    {
      pag_heap[pag_index_curr_pid] = i;
    }
  }
  pag_heap[pag_index_curr_pid]++;
  for (addr_t j = 0; j < pag_heap[pag_index_curr_pid] + 1; j++)
  {
    if (!(pag_page_table[pag_index_curr_pid][j] >= 0))
    {
      continue;
    }
    int flag = 1;
    int phisical_page = pag_page_table[pag_index_curr_pid][j];
    for (addr_t i = pag_get_base_of_page(phisical_page); i < pag_get_base_of_page(phisical_page) + pag_page_size; i++)
    {
      if (pag_virtual_m[i])
      {
        flag = 0;
        break;
      }
    }
    if (flag)
    {
      pag_free_list[phisical_page] = 1;
      pag_page_table[pag_index_curr_pid][j] = -1;
    }
  }
  return x;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  int vpn = pag_get_vpn(pag_stack[pag_index_curr_pid]);
  if (vpn < (int)pag_heap[pag_index_curr_pid])
  {
    return 1;
  }
  if (pag_page_table[pag_index_curr_pid][vpn] < 0)
  {
    int phisical_page = pag_free_page();

    if (phisical_page < 0)
    {
      return 1;
    }
    pag_page_table[pag_index_curr_pid][vpn] = phisical_page;
    pag_free_list[phisical_page] = 0;
  }

  pag_res_free_m(pag_stack[pag_index_curr_pid], 1, 1);

  m_write(pag_get_pa(pag_stack[pag_index_curr_pid]), val);
  out->addr = pag_stack[pag_index_curr_pid];
  pag_stack[pag_index_curr_pid]--;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (pag_stack[pag_index_curr_pid] + 1 > pag_pages_count * pag_page_size)
  {
    return 1;
  }
  pag_stack[pag_index_curr_pid]++;
  *out = m_read(pag_get_pa(pag_stack[pag_index_curr_pid]));
  size_t vpn = pag_get_vpn(pag_stack[pag_index_curr_pid] - 1);
  if (vpn != pag_get_vpn(pag_stack[pag_index_curr_pid]))
  {
    pag_free_list[pag_page_table[pag_index_curr_pid][vpn]] = 1;
    pag_page_table[pag_index_curr_pid][vpn] = -1;
  }
  pag_res_free_m(pag_stack[pag_index_curr_pid], 1, 0);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  if (pag_page_table[pag_index_curr_pid][pag_get_vpn(addr)] < 0)
  {
    return 1;
  }
  if (!pag_virtual_m[pag_get_pa(addr)])
  {
    return 1;
  }
  *out = m_read(pag_get_pa(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (pag_page_table[pag_index_curr_pid][pag_get_vpn(addr)] < 0)
  {
    return 1;
  }
  if (!pag_virtual_m[pag_get_pa(addr)])
  {
    return 1;
  }
  if (addr > pag_stack[pag_index_curr_pid])
  {
    return 1;
  }
  m_write(pag_get_pa(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  int aux = pag_index_curr_pid;
  pag_index_curr_pid = -1;
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (pag_pids[i] == process.pid)
    {
      pag_index_curr_pid = i;
      break;
    }
  }
  set_curr_owner(process.pid);

  if (pag_index_curr_pid < 0)
  {
    for (int i = 0; i < MAX_PROCS_COUNT; i++)
    {
      if (pag_pids[i] == -1)
      {
        pag_index_curr_pid = i;
        break;
      }
    }
    if (pag_index_curr_pid < 0)
    {
      pag_index_curr_pid = aux;
      set_curr_owner(pag_pids[pag_index_curr_pid]);
      return;
    }
    pag_pids[pag_index_curr_pid] = process.pid;
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  int temp = pag_index_curr_pid;
  pag_index_curr_pid = -1;
  for (int i = 0; i < MAX_PROCS_COUNT; i++)
  {
    if (pag_pids[i] == process.pid)
    {
      pag_index_curr_pid = i;
      break;
    }
  }

  if (pag_index_curr_pid < 0)
  {
    return;
  }
  ptr_t aux;
  for (int vpn = 0; vpn < pag_pages_count; vpn++)
  {
    if (pag_page_table[pag_index_curr_pid][vpn] >= 0)
    {
      aux.addr = vpn * pag_page_size;
      aux.size = pag_page_size;
      m_pag_free(aux);
    }
  }
  pag_pids[pag_index_curr_pid] = -1;
  pag_stack[pag_index_curr_pid] = pag_pages_count * pag_page_size - 1;
  pag_heap[pag_index_curr_pid] = 0;
  pag_index_curr_pid = temp;
}
