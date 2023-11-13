#include "pag_manager.h"

#include "stdio.h"

#include "../memory.h"

#ifndef VAL_CODE
#define VAL_CODE printf("Val_46")
#define fori(base, bound) for (size_t i = (size_t)base; i < (size_t)(base + bound); i++)
#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b
#define clamp(a, b, x) (x > b) ? b : (x < a) ? a \
                                             : x
#define MAX_PROC_COUNT 20
#endif

const int MASTER = 1;
int cero_messanges = 0; // 0 no allert

void set_messanges(int on)
{
  if (MASTER)
    return;
  cero_messanges = on;
}

// Context
int pag_cur_ipid;
int *pag_pids;
// 2 bits -> [No allocated] , 01 read[Stack], 10 write[Allocated and Heap], 11 read and write[Allocated and Heap]

// Proc
int *pag_heap;
addr_t *pag_stack;
int *pag_virtual_memory; // para el bug de free en el medio 0->Not USED  1-> USED

// Physical Memory
int *pag_free_page; // free list, ith pag is : 1->free 0->alloc
int pag_pages_count = 0;

// Virtual Memory (Espacio de direcciones virtuales)
int **pag_pages_table; // ith Procs pages table if page < 0 not valid

// VPN and PA [INPUTS]
size_t pag_page_size = 0;
size_t pag_offset_bits = 7; // 2^offset = page size

// FUNCTIONS
// PIDS
int pag_find_pid(int pid)
{
  fori(0, MAX_PROC_COUNT) if (pag_pids[i] == pid) return i;
  return -1;
}
int pag_free_pid()
{
  return pag_find_pid(-1);
}

// ADDRESSES

addr_t pag_get_base_page(int physical_page_number)
{
  return physical_page_number * pag_page_size;
}

addr_t pag_get_offset(addr_t addr)
{
  return addr & ((1 << pag_offset_bits) - 1);
}

addr_t pag_get_vpn(addr_t addr)
{
  return addr >> pag_offset_bits;
}

addr_t pag_get_pa(addr_t addr)
{
  int pp = pag_pages_table[pag_cur_ipid][pag_get_vpn(addr)];
  return pag_get_base_page(pp) + pag_get_offset(addr);
}

// PHYSICAL MEMORY

int pag_is_page_valid(int page_number)
{
  return pag_pages_table[pag_cur_ipid][page_number] >= 0;
}

void pag_update_heap()
{
  pag_heap[pag_cur_ipid] = -1;
  fori(0, pag_get_vpn(pag_stack[pag_cur_ipid])) if (pag_is_page_valid(i)) pag_heap[pag_cur_ipid] = i;
  pag_heap[pag_cur_ipid]++;
}

void pag_updated_valid_pages()
{
  pag_update_heap();
  for (int vpn = 0; vpn < pag_heap[pag_cur_ipid] + 1; vpn++)
  {
    if (!pag_is_page_valid(vpn))
      continue;

    int flag = 1;
    int pp = pag_pages_table[pag_cur_ipid][vpn];
    fori(pag_get_base_page(pp), pag_page_size)
    {
      if (pag_virtual_memory[i])
      {
        flag = 0;
        break;
      }
    }
    // printf("page: %d vpn: %d flag %d heap: %d ", pp, vpn, flag, pag_heap[pag_cur_ipid]);
    // exit(0);
    if (flag)
    {
      pag_free_page[pp] = 1;
      pag_pages_table[pag_cur_ipid][vpn] = -1;
    }
  }
}

int pag_alloc_free_memory(addr_t va, int while_x, int alloc)
{
  fori(va, while_x)
  {
    // printf("addr: %ld ", pag_get_pa(i));

    if (cero_messanges && ((alloc && pag_virtual_memory[pag_get_pa(i)]) || (!alloc && !pag_virtual_memory[pag_get_pa(i)])))
    {
      int a = pag_virtual_memory[pag_get_pa(i)];
      printf("ERROR in memory manager Alloc: %d, Virtual Memo:%d,  Addr: 0x%zx \n", alloc, a, pag_get_pa(i));
      return 1;
    }

    pag_virtual_memory[pag_get_pa(i)] = alloc;

    if (pag_get_vpn(i) != pag_get_vpn(va) && pag_get_pa(i) + 1 != pag_get_pa(i + 1)) // cambio de pagina
    {
      // printf("testing: %ld %ld", pag_get_pa(i), pag_get_pa(i + 1));
      if (alloc)
        m_set_owner(pag_get_pa(va), pag_get_pa(i - 1));
      else
        m_unset_owner(pag_get_pa(va), pag_get_pa(i - 1));
      va = i;
    }
  }

  if (alloc)
    m_set_owner(pag_get_pa(va), pag_get_pa(va + while_x - 1));
  else
    m_unset_owner(pag_get_pa(va), pag_get_pa(va + while_x - 1));

  pag_updated_valid_pages();
  return 0;
}

int pag_free_page_counting()
{
  int count = 0;
  fori(0, pag_pages_count) count += pag_free_page[i];
  return count;
}

int pag_find_free_page()
{
  fori(0, pag_pages_count) if (pag_free_page[i]) return i;
  return -1;
}

int _pag_find_free_continous_space(size_t size, addr_t *li)
{
  int free_pages_base = pag_free_page_counting();

  size_t free = 0;
  *li = __INT_MAX__;
  int free_pages = free_pages_base;

  for (size_t vpn = 0; vpn < pag_get_vpn(pag_stack[pag_cur_ipid]); vpn++)
  {
    if (pag_is_page_valid(vpn))
    {
      int pp = pag_pages_table[pag_cur_ipid][vpn];

      fori(pag_get_base_page(pp), pag_page_size)
      {
        free++;
        *li = min(*li, vpn * pag_page_size + i); // base en el espacio de direcciones

        if (pag_virtual_memory[i])
        {
          free = 0;
          *li = __INT_MAX__;
          free_pages = free_pages_base;
        }

        if (free >= size)
          return 0;
      }
    }

    else if (free_pages > 0)
    {
      free_pages--;
      free += pag_page_size;
      *li = min(*li, vpn * pag_page_size);

      if (free >= size)
        return 0;
    }
  }

  return -1;
}

int pag_malloc_size(size_t size)
{
  addr_t li;
  if (_pag_find_free_continous_space(size, &li) < 0)
  {
    printf("Continuous space not found\n");
    return -1;
  }

  size_t vpn = pag_get_vpn(li);

  while (vpn <= pag_get_vpn(li + size - 1))
  {
    if (!pag_is_page_valid(vpn))
    {
      int pfree_page = pag_find_free_page();
      pag_pages_table[pag_cur_ipid][vpn] = pfree_page;
      pag_free_page[pfree_page] = 0;
    }
    vpn++;
  }

  pag_alloc_free_memory(li, size, 1);
  return li;
}

// END FUNCTIONS

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  pag_cur_ipid = -1;
  pag_pids = (int *)malloc(sizeof(int) * MAX_PROC_COUNT);
  pag_stack = (addr_t *)malloc(sizeof(int) * MAX_PROC_COUNT);
  pag_heap = (int *)malloc(sizeof(int) * MAX_PROC_COUNT);

  pag_page_size = min(m_size(), (size_t)(1 << pag_offset_bits));
  pag_pages_count = m_size() / pag_page_size;

  pag_pages_table = (int **)malloc(sizeof(int) * MAX_PROC_COUNT);
  pag_free_page = (int *)malloc(sizeof(int) * pag_pages_count);

  fori(0, MAX_PROC_COUNT)
  {
    pag_pids[i] = -1;
    pag_stack[i] = pag_page_size * pag_pages_count - 1;
    pag_heap[i] = 0;

    pag_pages_table[i] = (int *)malloc(sizeof(int) * pag_pages_count);

    for (int vpn = 0; vpn < pag_pages_count; vpn++)
      pag_pages_table[i][vpn] = -1;
  }

  fori(0, pag_pages_count) pag_free_page[i] = 1;

  pag_virtual_memory = (int *)malloc(sizeof(int) * m_size());

  fori(0, m_size()) pag_virtual_memory[i] = 0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  pag_update_heap();
  int ans = pag_malloc_size(size);
  if (ans < 0)
  {
    printf("MALLOC HEAP: SIZE:%ld  ANS:%d\n", size, ans);
    return 1;
  }
  out->addr = ans;
  out->size = size;

  if (cero_messanges)
  {
    printf("Malloc virtual addr 0x%zx 0x%zx \n", out->addr, ((addr_t)(out->addr + out->size - 1)));
  }
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  size_t vpn = pag_get_vpn(ptr.addr);

  while (vpn <= pag_get_vpn(ptr.addr + ptr.size - 1))
  {
    if (!pag_is_page_valid(vpn))
    {
      return 1;
    }
    vpn++;
  }

  int ex = pag_alloc_free_memory(ptr.addr, ptr.size, 0);
  pag_updated_valid_pages();
  pag_update_heap();
  return ex;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  int vpn = pag_get_vpn(pag_stack[pag_cur_ipid]);
  if (vpn < pag_heap[pag_cur_ipid])
  {
    printf("PUSH ERROR: STACK OVERFLOW: Stack vpn:%d  Heap:%d\n", vpn, pag_heap[pag_cur_ipid]);
    return 1;
  }

  if (!pag_is_page_valid(vpn))
  {
    int pp = pag_find_free_page();

    if (pp < 0)
    {
      printf("MEMORY EXCEED IN STACK ALLOC");
      return 1;
    }

    // printf("PUSH > Resb a page for Stack vpn:%d\n", vpn);
    pag_pages_table[pag_cur_ipid][vpn] = pp;
    pag_free_page[pp] = 0;
  }

  pag_alloc_free_memory(pag_stack[pag_cur_ipid], 1, 1);

  m_write(pag_get_pa(pag_stack[pag_cur_ipid]), val);
  out->addr = pag_stack[pag_cur_ipid];
  pag_stack[pag_cur_ipid]--;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (pag_stack[pag_cur_ipid] + 1 > pag_pages_count * pag_page_size)
  {
    printf("INDEX OUT IN PAG POP: Stack: %ld ls: %ld", pag_stack[pag_cur_ipid] + 1, pag_pages_count * pag_page_size);
    return 1;
  }

  pag_stack[pag_cur_ipid]++;

  *out = m_read(pag_get_pa(pag_stack[pag_cur_ipid]));

  size_t vpn = pag_get_vpn(pag_stack[pag_cur_ipid] - 1);

  if (vpn != pag_get_vpn(pag_stack[pag_cur_ipid]))
  {
    pag_free_page[pag_pages_table[pag_cur_ipid][vpn]] = 1;
    pag_pages_table[pag_cur_ipid][vpn] = -1;
  }

  pag_alloc_free_memory(pag_stack[pag_cur_ipid], 1, 0);

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{

  if (!pag_is_page_valid(pag_get_vpn(addr)))
  {
    printf("PAG LOAD: NOT VALID ADDR: 0x%zx VPN: %ld VALID: %d\n", addr, pag_get_vpn(addr), pag_pages_table[pag_cur_ipid][pag_get_vpn(addr)]);
    return 1;
  }

  if (!pag_virtual_memory[pag_get_pa(addr)])
  {
    printf("PAG LOAD: NOT ALLOC MEMO : addr: 0x%zx  virtual Memo: %d\n", addr, pag_virtual_memory[pag_get_pa(addr)]);
    return 1;
  }

  *out = m_read(pag_get_pa(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (!pag_is_page_valid(pag_get_vpn(addr)))
  {
    printf("PAG STORE: NOT VALID ADDR: %ld VPN: %ld VALID: %d\n", addr, pag_get_vpn(addr), pag_pages_table[pag_cur_ipid][pag_get_vpn(addr)]);
    return 1;
  }

  if (!pag_virtual_memory[pag_get_pa(addr)])
  {
    printf("PAG STORE: NOT ALLOC MEMO : addr: 0x%zx  virtual Memo: %d\n", addr, pag_virtual_memory[pag_get_pa(addr)]);
    return 1;
  }

  if (addr > pag_stack[pag_cur_ipid])
  {
    printf("PAG STORE: Writing in stack: STACK: %ld ADDR: %ld\n", pag_stack[pag_cur_ipid], addr);
    return 1;
  }

  m_write(pag_get_pa(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  // printf("\n = = = = = = = = = = = = = = = = = = = =\n");
  int temp = pag_cur_ipid;
  pag_cur_ipid = pag_find_pid(process.pid);
  set_curr_owner(process.pid);

  if (pag_cur_ipid < 0)
  {
    pag_cur_ipid = pag_free_pid();

    if (pag_cur_ipid < 0)
    {
      printf("CAMBIO DE CONTEXTO: pid: %d No hay espacios libres\n", process.pid);
      pag_cur_ipid = temp;
      set_curr_owner(pag_pids[pag_cur_ipid]);
      return;
    }
    pag_pids[pag_cur_ipid] = process.pid;
  }
  if (cero_messanges)
  {
    printf("Cambio de Contexto %d\n", pag_pids[pag_cur_ipid]);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  int temp = pag_cur_ipid;
  pag_cur_ipid = pag_find_pid(process.pid);

  if (pag_cur_ipid < 0)
  {
    printf("ERROR => No se encontro el pid: %d\n", process.pid);
    return;
  }

  set_messanges(0);

  ptr_t aux;
  for (int vpn = 0; vpn < pag_pages_count; vpn++)
  {
    if (pag_is_page_valid(vpn))
    {
      aux.addr = vpn * pag_page_size;
      aux.size = pag_page_size;
      m_pag_free(aux);
      pag_free_page[pag_pages_table[pag_cur_ipid][vpn]] = 1;
    }
  }
  set_messanges(1);

  // delete procs func
  pag_pids[pag_cur_ipid] = -1;
  pag_stack[pag_cur_ipid] = pag_pages_count * pag_page_size - 1;
  pag_heap[pag_cur_ipid] = 0;
  pag_cur_ipid = temp;
}