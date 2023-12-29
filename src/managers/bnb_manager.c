#include "bnb_manager.h"

#include "stdio.h"

static addr_t curr_addres;
static int curr_proc_id;
static addr_t *proc_addres;

#define process_size 1024

typedef struct current_process //representa el estado actual de un proceso
{
  size_t size;       // tamaño del bloque de memoria
  int id;            // id del proceso
  addr_t base;       // direccion base  
  addr_t bound;     // direccion tope
  addr_t heap;       // puntero al inicio del heap
  addr_t stack;      // puntero al inicio del stack
  int exec;          // 1 si el proceso esta en ejecucion 
} current_process_t;

static current_process_t *blocks;  


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  free(proc_addres);
  free(blocks);
  size_t t_blocks = m_size()/process_size;
  
  curr_addres = 0;
  proc_addres = (size_t*)malloc(sizeof(size_t) * t_blocks);
  blocks = (current_process_t*)malloc(sizeof(current_process_t) * t_blocks);

  size_t start = 0;
  for(size_t i = 0; i < t_blocks; i++)
  {
    start += process_size;
    current_process_t *current_block = &blocks[i];
    current_block->base = start + 1;
    current_block->size = 0;
    current_block->bound = start + process_size - 1;
    current_block->id = NO_ONWER;
    current_block->heap = start + 1;
    current_block->stack = start + process_size - 1;
    current_block->exec = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  for(size_t i = 0; i < m_size() / process_size; i++)
  {
    if(!blocks[i].exec)
    {     
      size_t tp = i * process_size;
      m_set_owner(tp + 1, tp + process_size - 1);

      curr_addres = i;
      proc_addres[curr_proc_id] = i;
      blocks[i].size = size;
      blocks[i].exec = 1;
      blocks[i].id = curr_proc_id;

      out->size = 1;
      out->addr = tp + 1;

      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  size_t base = blocks[curr_addres].base;
  size_t bound = blocks[curr_addres].bound;

  if(ptr.addr >= base && ptr.addr + ptr.size < bound)
  {
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
    blocks[curr_addres].size -= ptr.size;
    return 0;
  }
  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  if(blocks[curr_addres].stack - 1 <= blocks[curr_addres].heap)
  {
    return 1;
  }

  m_write(blocks[curr_addres].stack, val);
  blocks[curr_addres].stack --;
  out->addr = blocks[curr_addres].stack;

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  addr_t base = blocks[curr_addres].base;
  addr_t bound = blocks[curr_addres].bound;
  addr_t stack_top = blocks[curr_addres].stack + 1;
  
  if(base + bound <= stack_top)
  {
    return 1;
  }

  *out = m_read(stack_top);
  blocks[curr_addres].stack ++;
  
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  addr_t base = blocks[curr_addres].base;
  addr_t bound = blocks[curr_addres].bound;

  if(addr >= base && addr < bound)
  {
    *out = m_read(addr);

    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  addr_t base = blocks[curr_addres].base;
  addr_t size = blocks[curr_addres].size;

  if(addr >= base && addr < base + size)
  {
    m_write(addr, val);

    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  curr_proc_id = process.pid;

  curr_addres = proc_addres[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  addr_t address = proc_addres[process.pid];

  m_unset_owner(blocks[address].base, blocks[address].bound);

  blocks[address].size = 0;
  blocks[address].id = NO_ONWER;
  blocks[address].exec = 0;
  blocks[address].heap = blocks[address].base;
  blocks[address].stack = blocks[address].bound;
}
