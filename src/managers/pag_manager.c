#include "pag_manager.h"

#include "stdio.h"

#include "../memory.h"

#define MAX_PROC_COUNT 20
#define forn(a, b) for(addr_t i = (addr_t)a; i < (addr_t)b; i++)
#define min(a, b) (a < b) ? a : b;

int **pag_table;

int *pag_pid;
int *pag_free_memory;
int *pag_free_page;
addr_t *pag_stack;

int pag_ind;
int pag_pages_count = 0;
addr_t pag_page_size = 0;
addr_t pag_offset_bits = 6;

addr_t pag_get_offset(addr_t addr)
{
  return addr & ((1 << pag_offset_bits) - 1);
}

addr_t pag_get_vpn(addr_t addr)
{
  return addr >> pag_offset_bits;
}

int page_is_valid(int vpn)
{
  return pag_table[pag_ind][vpn] >= 0;
}

int pag_find_pid(int _pid)
{
  forn(0, MAX_PROC_COUNT)
  {
    if(_pid == pag_pid[i]) return i;
  }
  return pag_find_pid(-1);
}


addr_t get_free_adress()
{
  forn(0, pag_pages_count)
  {
    if(pag_free_page[i] > 0)
    {
      pag_free_page[i] = 0;
      
      m_set_owner(i * pag_page_size, (i + 1) * pag_page_size - 1);
      
      return i;
    }
  }
  return -1;
}

int find_continuous_spaces(int size, addr_t *start)
{
  int ind = pag_ind;
  int c_size = size;

  for(addr_t vpn = 0; vpn < pag_stack[ind]; vpn++)
  {
    if(page_is_valid(vpn))
    {
      *start = vpn + 1;
      c_size = size;
      continue;
    }
    
    c_size -= pag_page_size;
    
    if(c_size < 0) return 0;
  }

  return 1;
}


addr_t try_alloc(int size)
{
  addr_t start;

  if(find_continuous_spaces(size, &start))
  {
    printf("Can't allocate spaces");
    return -1;
  }

  set_curr_owner(pag_pid[pag_ind]);

  forn(start, start + size)
  {
    pag_table[pag_ind][i] = get_free_adress();
  }

  return start;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{

  pag_ind = -1;
  pag_pid = (int *)malloc(sizeof(int) * MAX_PROC_COUNT);
  pag_stack = (addr_t *)malloc(sizeof(addr_t) * MAX_PROC_COUNT);

  pag_page_size = min(m_size(), (size_t)(1 << pag_offset_bits));
  pag_pages_count = m_size() / pag_page_size;
  pag_free_memory = (int *)malloc(sizeof(int) * m_size());

  forn(0, MAX_PROC_COUNT)
  {
    pag_pid[i] = -1;
    pag_stack[i] = pag_page_size * pag_pages_count - 1;

    pag_table[i] = (int *)malloc(sizeof(int) * pag_pages_count);

    for (int j = 0; j < pag_pages_count; j++)
    {
      pag_table[i][j] = -1;
    }
  }

  forn(0, pag_pages_count) pag_free_page[i] = 1;
  forn(0, m_size()) pag_free_memory[i] = 0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  int addr = try_alloc(size);
  if(addr < 0)
  {
    printf("MALLOC ERROR");
    return 1;
  }

  out->addr = addr;
  out->size = size;

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  pag_ind = pag_find_pid(process.pid);
  pag_pid[pag_ind] = process.pid;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
