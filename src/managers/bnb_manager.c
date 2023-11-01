#include "bnb_manager.h"

#include "stdio.h"

static memory_block *virtual_memory;
static size_t *process_addr;
static size_t actual_start_addr;
static int cant_blocks;
static int actual_process_pid;

#define BLOCK_SIZE 1024
#define CODE_SIZE 1
#define TO_KB(size) size / BLOCK_SIZE


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  /* Libero la memoria en caso de estar reservadas previamente */
  if (virtual_memory != NULL) {
    free(virtual_memory);
    virtual_memory = NULL;
  }
  if (process_addr != NULL) {
    free(process_addr);
    process_addr = NULL;
  }

  size_t num_blocks = TO_KB(m_size());

  virtual_memory = (memory_block*)malloc(num_blocks * sizeof(memory_block));
  process_addr = (size_t*)malloc(num_blocks * sizeof(size_t));
  actual_start_addr = 0;

  for (size_t i = 0, start = 0; i < num_blocks; i++, start += BLOCK_SIZE) {

    /* Reservo espacio para el codigo */
    virtual_memory[i].start_addr = start + CODE_SIZE;
    virtual_memory[i].end_addr = start + BLOCK_SIZE - 1;
    virtual_memory[i].heap = start + CODE_SIZE;
    virtual_memory[i].stack = start + BLOCK_SIZE - 1;

    virtual_memory[i].is_allocated = 0;
    virtual_memory[i].owner = NO_ONWER;
    virtual_memory[i].size = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  int found = 1;

  // Busca un bloque libre de tamanio mayor o igual que size, lo reserva con m_set_owner
  for (size_t i = 0, addr = 0; addr < m_size(); i ++, addr += BLOCK_SIZE) {
    if (!virtual_memory[i].is_allocated) {
      m_set_owner(addr, addr + BLOCK_SIZE - 1);
      found = 0;
      
      // Actualiza los valores del espacio reservado
      process_addr[actual_process_pid] = i;
      actual_start_addr = i;

      virtual_memory[i].is_allocated = 1;
      virtual_memory[i].owner        = actual_process_pid;
      virtual_memory[i].size         = size;
      
      out->addr = addr + CODE_SIZE;
      out->size = 1;

      break;
    }
  }

  return found;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int STATE = 1;
  size_t start_addr = virtual_memory[actual_start_addr].start_addr;
  size_t actual_size_addr  = virtual_memory[actual_start_addr].size;

  // Si ptr se encuentra dentro del rango reservado por el proceso actual
  if (start_addr <= ptr.addr && ptr.addr + ptr.size < start_addr + actual_size_addr) {
    STATE = 0;

    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
    
    virtual_memory[start_addr].size -= ptr.size;    
  }
  
  return STATE;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {  
  size_t actual_size_addr  = virtual_memory[actual_start_addr].size;

  if (virtual_memory[actual_process_pid].stack - 1 <= virtual_memory[actual_process_pid].heap) {
    return 1;
  }

  m_write(virtual_memory[actual_start_addr].stack, val);
  virtual_memory[actual_start_addr].stack -= 1;
  
  out->addr = virtual_memory[actual_start_addr].stack;

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  size_t LIMIT_BLOCK  = virtual_memory[actual_start_addr].end_addr;
  size_t start_addr  = virtual_memory[actual_start_addr].start_addr;

  if (virtual_memory[actual_process_pid].stack + 1 >= start_addr + LIMIT_BLOCK) {
    return 1;
  }

  *out = m_read(virtual_memory[actual_start_addr].stack + 1);
  virtual_memory[actual_start_addr].stack += 1;

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  size_t actual_size_addr  = virtual_memory[actual_start_addr].size;
  size_t start_addr  = virtual_memory[actual_start_addr].start_addr;

  if (addr >= start_addr && addr < start_addr + actual_size_addr) {
    *out = m_read(addr);

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  size_t actual_size_addr  = virtual_memory[actual_start_addr].size;
  size_t start_addr  = virtual_memory[actual_start_addr].start_addr;

  if (addr >= start_addr && addr < start_addr + actual_size_addr) {
    m_write(addr, val);
    
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  actual_process_pid = process.pid;
  actual_start_addr = process_addr[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  size_t addr = process_addr[process.pid];
  size_t size = virtual_memory[addr].size;
  size_t start_addr  = virtual_memory[actual_start_addr].start_addr;
  size_t end_addr  = virtual_memory[actual_start_addr].end_addr;
  
  m_unset_owner(start_addr, end_addr);

  virtual_memory[addr].heap = start_addr;
  virtual_memory[addr].stack = end_addr;

  virtual_memory[addr].is_allocated = 0;
  virtual_memory[addr].owner = NO_ONWER;
  virtual_memory[addr].size = 0;
}
