#include "bnb_manager.h"

#include "stdio.h"

#include "../memory.h"

#define MAX_PROC_COUNT 20
#define forn(a, b) for(addr_t i = a; i < b; i++) 

int curr_ind = -1;

int *pid; 
int *free_list;

addr_t *heap;
addr_t *stack;
addr_t bound;
addr_t base;

int is_valid(addr_t addr)
{
  printf("FUERA DE RANGO");
  return addr > bound;
}

addr_t get_address(int i, addr_t offset)
{
  return base * i + offset;
}

int check_addr(addr_t addr)
{
  if(is_valid(addr)) return 1;

  if(free_list[get_address(curr_ind, addr)] < 0)
  {
    printf("MEMORIA NO ASIGNADA");
    return 1;
  }

  return 0;
}

void alloc_mem(addr_t addr, int size, int set_pid)
{
  forn(addr, addr + size)
  {
    free_list[i] = set_pid;
  }
}

int set_owner(int i)
{
  m_set_owner(get_address(i, 0), get_address(i, bound - 1));

  return i;
}

int find_pid(int _pid)
{
  forn(0, MAX_PROC_COUNT)
  {
    if(_pid == pid[i]) return i;
  }
  return set_owner(find_pid(-1));
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) 
{
  bound = m_size() / MAX_PROC_COUNT;

  curr_ind = -1;
  pid = malloc(sizeof(int) * MAX_PROC_COUNT);
  heap = malloc(sizeof(addr_t) * MAX_PROC_COUNT);
  stack = malloc(sizeof(addr_t) * MAX_PROC_COUNT);
  free_list = malloc(sizeof(int) * m_size());

  forn(0, MAX_PROC_COUNT)
  {
    pid[i] = -1;
    heap[i] = 0;
    stack[i] = bound;
  }

  forn(0, m_size())
  {
    free_list[i] = -1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(addr_t size, ptr_t *out) 
{
  int i = curr_ind;

  if(heap[i] + size > stack[i])
  {
    printf("MALLOC NOT ENOUGH SPACE");
    return 1;
  }

  alloc_mem(get_address(i , heap[i]), size, pid[i]);
  
  out->addr = heap[i];
  out->size = size;

  heap[i] += size;

  return 0; 
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) 
{
  if(check_addr(ptr.addr))
  {
    printf("FREE ERROR");
    return 1;
  }

  alloc_mem(get_address(curr_ind, ptr.addr), ptr.size, -1);

  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) 
{
  int i = curr_ind;
  
  if(heap[i] >= stack[i])
  {
    printf("STACK OVERFLOW");
    return 1;
  }

  stack[i] -= 1;
  out->addr = stack[i];
  out->size = 1;

  m_write(get_address(i, stack[i]), val);

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) 
{
  int i = curr_ind;
  if(check_addr(stack[i] + 1))
  {
    printf("NO ITEM TO POP");
    return 1;
  }

  stack[i] += 1;
  *out = m_read(get_address(i, stack[i]));

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) 
{
  if(check_addr(addr)) return 1;

  *out = m_read(get_address(curr_ind, addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) 
{
  if(check_addr(addr)) return 1;

  m_write(get_address(curr_ind, addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) 
{  
  set_curr_owner(process.pid);
  curr_ind = find_pid(process.pid);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) 
{  
  set_curr_owner(-1);
  set_owner(find_pid(process.pid));
}
