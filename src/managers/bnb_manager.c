#include "bnb_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define Block_Size 1024
#define Code_Size 1
#define kb(size) ((size) / Block_Size)
static int current_pid;      
static size_t current_addr;  
static size_t *process_addr; 
static Block *memory_v;
static FreeBlock* free_blocks = NULL;

void initialize_memory_block(Block *block, size_t start) {
  size_t end = start + Block_Size - 1;
  block->heap = start + Code_Size;
  block->stack = end;
  block->s_addr = start + Code_Size;
  block->e_addr = end;
  block->size = 0;
  block->in_use = 0;
  block->owner = NO_ONWER;
}

void reset_memory_block(Block *block) {
  block->in_use = 0;
  block->owner = NO_ONWER;
  block->size = 0;
  block->heap = block->s_addr;
  block->stack = block->e_addr;
}

// Función para inicializar la memoria.
void m_bnb_init(int argc, char **argv) {
  free(memory_v);
  free(process_addr);
  size_t cant_block = kb(m_size());

  memory_v = (Block *)malloc(sizeof(Block) * cant_block);
  process_addr = (size_t *)malloc(sizeof(size_t) * cant_block);
  current_addr = 0;

  FreeBlock* initial_block = (FreeBlock*)malloc(sizeof(FreeBlock));
  initial_block->start_addr = 0;
  initial_block->end_addr = cant_block * Block_Size - 1;
  initial_block->next = NULL;
  free_blocks = initial_block;

  for (size_t i = 0, start = 0; i < cant_block; i++, start += Block_Size) {
    Block *currentBlock = &memory_v[i];
    size_t end = start + Block_Size - 1;
    initialize_memory_block( currentBlock, start);
  }
}

// Función para asignar memoria (malloc).
int m_bnb_malloc(size_t size, ptr_t *out) {
  size_t total_blocks = kb(m_size());

  for (size_t i = 0; i < total_blocks; i++) {
    Block *currentBlock = &memory_v[i];

    if (!currentBlock->in_use) {  // Encuentra un bloque no utilizado.
      size_t block_start = i * Block_Size + Code_Size;
      size_t block_end = i * Block_Size + Block_Size - 1;

      // Establece el propietario y asigna la dirección del proceso.
      m_set_owner(block_start, block_end);
      process_addr[current_pid] = i;
      current_addr = i;

      // Actualiza el bloque de memoria.
      currentBlock->in_use = 1;
      currentBlock->owner = current_pid;
      currentBlock->size = size;

      // Asigna la dirección de inicio del bloque a la salida.
      out->addr = block_start;
      out->size = 1;  // Tamaño de 1 bloque.

      // Actualiza la lista de bloques libres:
      FreeBlock* current = free_blocks;
      FreeBlock* prev = NULL;

      while (current) {
        if (current->start_addr == block_start) {
          if (current->start_addr + current->end_addr == block_end) {
            // El bloque libre coincide exactamente con el bloque asignado, eliminarlo.
            if (prev) {
              prev->next = current->next;
            } else {
              free_blocks = current->next;
            }
            free(current);
          } else {
            // El bloque libre es parte del bloque asignado, ajustar su dirección de inicio.
            current->start_addr = block_start + size;
          }
          break;
        }
        prev = current;
        current = current->next;
      }

      return 0; 
    }
  }
  return 1;
}


// Función para liberar memoria (free).
int m_bnb_free(ptr_t ptr) {
  Block* currentBlock = &memory_v[current_addr];
  size_t start = currentBlock->s_addr;
  size_t end = currentBlock->e_addr;

  if (ptr.addr >= start && ptr.addr + ptr.size < end) {
    currentBlock->size -= ptr.size; 
    // Agregar el bloque liberado a la lista de bloques libres:
    FreeBlock* new_free_block = (FreeBlock*)malloc(sizeof(FreeBlock));
    new_free_block->start_addr = ptr.addr;
    new_free_block->end_addr = ptr.addr + ptr.size - 1;
    new_free_block->next = free_blocks;
    free_blocks = new_free_block;

    return 0; 
  }
  return 1;  
}


// Función para empujar un valor en la pila.
int m_bnb_push(byte val, ptr_t *out) {
  Block *currentBlock = &memory_v[current_addr];
  size_t stack_top = currentBlock->stack;

  if (stack_top <= currentBlock->heap) {
    return 1;
  }

  m_write(stack_top, val);
  currentBlock->stack = stack_top - 1;
  out->addr = stack_top;
  return 0; 
}



// Función para sacar un valor de la pila.
int m_bnb_pop(byte *out) {
  Block *currentBlock = &memory_v[current_addr];
  size_t stack_top = currentBlock->stack + 1;

  if (stack_top > currentBlock->e_addr) {
    return 1;
  }

  *out = m_read(stack_top);
  currentBlock->stack = stack_top;
  return 0; 
}


// Función para cargar un byte desde una dirección.
int m_bnb_load(addr_t addr, byte *out) {
  Block *currentBlock = &memory_v[current_addr];

  if (addr >= currentBlock->s_addr && addr < currentBlock->e_addr) {
    //load from
    *out = m_read(addr);
    return 0; 
  }

  return 1;
}


// Función para almacenar un byte en una dirección.
int m_bnb_store(addr_t address, byte value) {
  Block *currentBlock = &memory_v[current_addr];

  if (address >= currentBlock->s_addr && address < currentBlock->s_addr + currentBlock->size) {
    //write at
    m_write(address, value);
    return 0; 
  }

  return 1;
}


// Función para manejar el cambio de contexto de un proceso.
void m_bnb_on_ctx_switch(process_t process) {
  //simple update
  current_pid = process.pid;
  current_addr = process_addr[current_pid];
}


// Función para manejar el final de un proceso.
void m_bnb_on_end_process(process_t process) {
  size_t addr = process_addr[process.pid];
  m_unset_owner(memory_v[addr].s_addr, memory_v[addr].e_addr);
  Block *currentBlock = &memory_v[addr];

  reset_memory_block(currentBlock);
}



