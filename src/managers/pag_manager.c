#include "pag_manager.h"

#include "stdio.h"

#include "../memory.h"

#ifndef VAL_CODE
#define VAL_CODE printf("Val_46")
#define fori(base, bound) for (size_t i = (size_t)base; i < (size_t)base + bound; i++)
#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b
#define clamp(a, b, x) (x > b) ? b : (x < a) ? a \
                                             : x
#define MAX_PROC_COUNT 20
#endif

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
int pag_pages_count;

// Virtual Memory (Espacio de direcciones virtuales)
int **pag_pages_table; // ith Procs pages table
int **pag_pages_valid; // ith 1->valid 0->not valid

// VPN and PA [INPUTS]
size_t pag_page_size;
size_t pag_offset_bits = 4; // 2^offset = page size

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

int pag_get_offset(addr_t addr)
{
  return addr & ((1 << pag_offset_bits) - 1);
}

int pag_get_vpn(addr_t addr)
{
  return addr >> pag_offset_bits;
}

addr_t pag_get_pa(addr_t addr)
{
  int pp = pag_pages_table[pag_cur_ipid][pag_get_vpn(addr)];
  return pag_get_base_page(pp) + pag_get_offset(addr);
}

// PHYSICAL MEMORY

void pag_update_heap()
{
  fori(0, pag_get_vpn(pag_stack[pag_cur_ipid])) if (pag_pages_valid[pag_cur_ipid][i]) pag_heap[pag_cur_ipid] = i;
}

void pag_updated_valid_pages()
{
  for (int vpn = 0; vpn < pag_heap[pag_cur_ipid] + 1; vpn++)
  {
    if (!pag_pages_valid[pag_cur_ipid][vpn])
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
    if (flag)
    {
      pag_free_page[pp] = 1;
      pag_pages_valid[pag_cur_ipid][vpn] = 0;
    }
  }
  pag_update_heap();
}

int pag_alloc_free_memory(addr_t paddress, int while_x, int alloc)
{
  printf("Alloc...:\n");
  fori(paddress, while_x)
  {
    if ((alloc && pag_virtual_memory[i]) || (!alloc && !pag_virtual_memory[i]))
    {
      printf("ERROR in memory manager Alloc?: %d,  is memory used?: %d\n", alloc, pag_virtual_memory[i]);
      return 1;
    }
    printf("0x%zx ", i);
    pag_virtual_memory[i] = alloc;
  }
  printf("\n");
  if (alloc)
    m_set_owner(paddress, paddress + while_x - 1);

  else
    m_unset_owner(paddress, paddress + while_x - 1);

  // pag_updated_valid_pages();
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
  printf("free_pages_base: %d\n", free_pages_base);
  size_t free = 0;
  *li = __INT_MAX__;
  int free_pages = free_pages_base;

  for (int vpn = 0; vpn < pag_pages_count; vpn++)
  {
    if (pag_pages_valid[pag_cur_ipid][vpn])
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

int pag_malloc_size(size_t size, ptr_t *ptr)
{
  addr_t li;
  if (_pag_find_free_continous_space(size, &li) < 0)
  {
    printf("Continuous space not found\n");
    return 1;
  }

  int vpn = pag_get_vpn(li);

  while (vpn <= pag_get_vpn(li + size))
  {
    if (!pag_pages_valid[pag_cur_ipid][vpn])
    {
      pag_pages_valid[pag_cur_ipid][vpn] = 1;

      int pfree_page = pag_find_free_page();
      pag_pages_table[pag_cur_ipid][vpn] = pfree_page;
      pag_free_page[pfree_page] = 0;
    }
    vpn++;
  }
  printf("Found memory va: 0x%zx -> 0x%zx  physical addr: page %d 0x%zx -> 0x%zx\n", li, li + size - 1, pag_pages_table[pag_cur_ipid][pag_get_vpn(li)], pag_get_pa(li), pag_get_pa(li + size - 1));
  pag_alloc_free_memory(pag_get_pa(li), size, 1);

  ptr->addr = li;
  pag_update_heap();
  return 0;
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
  pag_pages_valid = (int **)malloc(sizeof(int) * MAX_PROC_COUNT);
  pag_free_page = (int *)malloc(sizeof(int) * pag_pages_count);

  fori(0, MAX_PROC_COUNT)
  {
    pag_pids[i] = -1;
    pag_stack[i] = pag_page_size * pag_pages_count - 1;
    pag_heap[i] = 0;

    pag_pages_table[i] = (int *)malloc(sizeof(int) * pag_pages_count);
    pag_pages_valid[i] = (int *)malloc(sizeof(int) * pag_pages_count);

    for (int vpn = 0; vpn < pag_pages_count; vpn++)
    {
      pag_pages_valid[i][vpn] = 0;
    }
  }

  fori(0, pag_pages_count) pag_free_page[i] = 1;

  pag_virtual_memory = (int *)malloc(sizeof(int) * m_size());

  fori(0, m_size()) pag_virtual_memory[i] = 0;

  printf(" = = = = = = = = = = = = = = = = = = = =\n");
  printf("PAG INIT\n");
  printf("pag_pages_count: %d\n", pag_pages_count);
  printf("pag_page_size: %ld\n", pag_page_size);
  printf("pag_offset_bits: %ld\n", pag_offset_bits);
  printf("pag_free_page_counting: %d\n", pag_free_page_counting());
  printf(" = = = = = = = = = = = = = = = = = = = =\n");
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  int ans = pag_malloc_size(size, out);
  if (ans)
  {
    printf("MALLOC HEAP > SIZE:%ld  ANS:%d\n", size, ans);
    return 1;
  }
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  int vpn = pag_get_vpn(ptr.addr);

  while (vpn <= pag_get_vpn(ptr.addr + ptr.size))
  {
    if (!pag_pages_valid[pag_cur_ipid][vpn])
      return 1;
    vpn++;
  }

  int ex = pag_alloc_free_memory(pag_get_pa(ptr.addr), ptr.size, 0);
  pag_updated_valid_pages();
  return ex;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  int vpn = pag_get_vpn(pag_stack[pag_cur_ipid]);
  if (vpn < pag_heap[pag_cur_ipid])
  {
    printf("PUSH ERROR > STACK OVERFLOW: Stack vpn:%d  Heap:%d\n", vpn, pag_heap[pag_cur_ipid]);
    return 1;
  }

  if (!pag_pages_valid[pag_cur_ipid][vpn])
  {
    int pp = pag_find_free_page();

    if (pp < 0)
    {
      printf("MEMORY EXCEED IN STACK ALLOC");
      return 1;
    }

    printf("PUSH > Resb a page for Stack vpn:%d\n", vpn);
    pag_pages_valid[pag_cur_ipid][vpn] = 1;
    pag_pages_table[pag_cur_ipid][vpn] = pp;
    pag_free_page[pp] = 0;
    printf("PUSH > Resb a physical page for Stack pa: %d 0x%zx -> 0x%zx\n", pp, pag_get_base_page(pp), pag_get_base_page(pp) + pag_page_size - 1);
    pag_alloc_free_memory(pag_get_base_page(pp), pag_page_size, 1);
  }

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
    printf("INDEX OUT IN PAG POP > Stack: %ld LS: %ld", pag_stack[pag_cur_ipid] + 1, pag_pages_count * pag_page_size);
    return 1;
  }

  pag_stack[pag_cur_ipid]++;
  *out = m_read(pag_get_pa(pag_stack[pag_cur_ipid]));

  int vpn = pag_get_vpn(pag_stack[pag_cur_ipid] - 1);

  if (vpn != pag_get_vpn(pag_stack[pag_cur_ipid]))
  {
    pag_pages_valid[pag_cur_ipid][vpn] = 0;
    pag_free_page[pag_pages_table[pag_cur_ipid][vpn]] = 1;
    pag_alloc_free_memory(pag_get_base_page(pag_get_vpn(pag_stack[pag_cur_ipid] - 1)), pag_page_size, 0);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{

  if (!pag_pages_valid[pag_cur_ipid][pag_get_vpn(addr)])
  {
    printf("PAG LOAD > NOT VALID ADDR: 0x%zx VPN: %d VALID: %d\n", addr, pag_get_vpn(addr), pag_pages_valid[pag_cur_ipid][pag_get_vpn(addr)]);
    return 1;
  }

  if (!pag_virtual_memory[pag_get_pa(addr)])
  {
    printf("PAG LOAD > NOT ALLOC MEMO : addr: 0x%zx  virtual Memo: %d\n", addr, pag_virtual_memory[pag_get_pa(addr)]);
    return 1;
  }

  *out = m_read(pag_get_pa(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (!pag_pages_valid[pag_cur_ipid][pag_get_vpn(addr)])
  {
    printf("PAG STORE > NOT VALID ADDR: %ld VPN: %d VALID: %d\n", addr, pag_get_vpn(addr), pag_pages_valid[pag_cur_ipid][pag_get_vpn(addr)]);
    return 1;
  }

  if (!pag_virtual_memory[pag_get_pa(addr)])
  {
    printf("PAG STORE > NOT ALLOC MEMO : addr: 0x%zx  virtual Memo: %d\n", addr, pag_virtual_memory[pag_get_pa(addr)]);
    return 1;
  }

  if (addr > pag_stack[pag_cur_ipid])
  {
    printf("PAG STORE > Writing in stack: STACK: %ld ADDR: %ld\n", pag_stack[pag_cur_ipid], addr);
    return 1;
  }

  m_write(pag_get_pa(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  printf("\n = = = = = = = = = = = = = = = = = = = =\n");
  int index = pag_find_pid(process.pid);

  if (index < 0)
  {
    index = pag_free_pid();

    if (index < 0)
    {
      printf("CAMBIO DE CONTEXTO: pid: %d No hay espacios libres\n", process.pid);
      return;
    }

    pag_pids[index] = process.pid;
  }

  pag_cur_ipid = index;
  printf("CAMBIO DE CONTEXTO > pid:%d, Index: %d Check: %d\n", process.pid, pag_cur_ipid, pag_pids[pag_cur_ipid]);
  (
      printf("Stats: Stack: 0%zx  Heap: %d\n", pag_stack[pag_cur_ipid], pag_heap[pag_cur_ipid]));
  set_curr_owner(process.pid);
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

  int heap_vpn = pag_heap[pag_cur_ipid];
  printf("heap_vpn: %d\n", heap_vpn);
  while (heap_vpn >= 0)
  {
    if (pag_pages_valid[heap_vpn])
    {
      pag_pages_valid[heap_vpn] = 0;
      pag_free_page[heap_vpn] = 1;
    }
    heap_vpn--;
  }

  printf("pag_cur_ipid: %d\n", pag_cur_ipid);
  int stack_vpn = pag_get_vpn(pag_stack[pag_cur_ipid]);

  while (stack_vpn < pag_pages_count)
  {
    if (pag_pages_valid[stack_vpn])
    {
      pag_pages_valid[stack_vpn] = 0;
      pag_free_page[stack_vpn] = 1;
    }
    stack_vpn++;
  }

  // delete procs func
  pag_pids[pag_cur_ipid] = -1;
  pag_stack[pag_cur_ipid] = pag_pages_count * pag_page_size - 1;
  pag_heap[pag_cur_ipid] = 0;

  pag_cur_ipid = temp;
}