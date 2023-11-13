#include "bnb_manager.h"

#include "stdio.h"

static int curr_pid;       // ID del proceso actual
static addr_t curr_addr;   // Dirección actual
static addr_t *procs_addr; // direcciones de procesos
static mem_block_t *v_mem; // bloques de memoria

#define block_sz 1024
#define code_sz 1
#define kb(size) ((size) / block_sz)

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  if (v_mem != NULL)
  {
    free(v_mem); // Libera la memoria
    v_mem = NULL;
  }

  if (procs_addr != NULL)
  {
    free(procs_addr); // Libera las direcciones de procesos
    procs_addr = NULL;
  }

  size_t block_count = kb(m_size());

  v_mem = (mem_block_t *)malloc(sizeof(mem_block_t) * block_count);
  procs_addr = (size_t *)malloc(sizeof(size_t) * block_count);
  curr_addr = 0;

  for (size_t i = 0, start = 0; i < block_count; i++, start += block_sz)
  {
    mem_block_t *curr_block = &v_mem[i];
    curr_block->heap = start + code_sz;
    curr_block->stack = start + block_sz - 1;
    curr_block->st = start + code_sz;
    curr_block->end = start + block_sz - 1;
    curr_block->sz = 0;
    curr_block->on_use = 0;
    curr_block->user = NO_ONWER;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  for (size_t i = 0; i < kb(m_size()); i++)
  {
    if (!v_mem[i].on_use)
    {
      size_t offset = i * block_sz;
      m_set_owner(offset + code_sz, offset + block_sz - 1);
      // llenar datos de procesos y memoria
      procs_addr[curr_pid] = i;
      curr_addr = i;
      v_mem[i].on_use = 1;
      v_mem[i].user = curr_pid;
      v_mem[i].sz = size;

      // llenar la salida
      out->addr = offset + code_sz;
      out->size = 1;
      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  size_t start = v_mem[curr_addr].st;
  size_t end = v_mem[curr_addr].end;

  if (ptr.addr >= start && ptr.addr + ptr.size < end)
  { // Comprueba si la dirección pertenece al bloque actual.
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
    v_mem[curr_addr].sz -= ptr.size;
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (v_mem[curr_addr].stack - 1 <= v_mem[curr_addr].heap)
  {
    return 1; // pila llena
  }

  // agrega el dato a la pila y cambia la direccion
  m_write(v_mem[curr_addr].stack, val);
  v_mem[curr_addr].stack--;
  out->addr = v_mem[curr_addr].stack;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  addr_t stack_top = v_mem[curr_addr].stack + 1;
  addr_t ini_bloque = v_mem[curr_addr].st;
  addr_t fin_bloque = v_mem[curr_addr].end;

  if (ini_bloque + fin_bloque <= stack_top)
  {
    return 1;
  }

  *out = m_read(stack_top); // Lee el valor en la cima de la pila
  v_mem[curr_addr].stack++; // Actualiza la posición de la pila
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  addr_t start = v_mem[curr_addr].st;
  addr_t end = v_mem[curr_addr].end;

  if (addr >= start && addr < end)
  { // Comprueba si la dirección pertenece al bloque actual.
    *out = m_read(addr);
    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  addr_t star_addr = v_mem[curr_addr].st;
  addr_t curr_sz = v_mem[curr_addr].sz;

  if (addr >= star_addr && addr < star_addr + curr_sz)
  { // Comprueba si la dirección pertenece al bloque actual.
    m_write(addr, val);
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  curr_pid = process.pid; // Actualiza el pid del proceso actual.
  curr_addr = procs_addr[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  addr_t addr = procs_addr[process.pid]; // Obtiene la dirección del proceso.
  m_unset_owner(v_mem[addr].st, v_mem[addr].end);
  v_mem[addr].on_use = 0;
  v_mem[addr].user = NO_ONWER;
  v_mem[addr].sz = 0;
  v_mem[addr].heap = v_mem[addr].st;
  v_mem[addr].stack = v_mem[addr].end;
}
