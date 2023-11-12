#include "bnb_manager.h"
#include <stdio.h>
#include <stdlib.h>

static int actual_Pid;      // ID del proceso actual.
static size_t actual_addr;  // Dirección actual.
static size_t *process_addr; // Arreglo de direcciones de procesos.
static Block *memory_v;      // Arreglo de bloques de memoria.

typedef struct FreeBlock {
  size_t start_addr;
  size_t end_addr;
  struct FreeBlock* next;
} FreeBlock;

static FreeBlock* free_blocks = NULL;

#define Block_Size 1024
#define Code_Size 1
#define Kb(size) ((size) / Block_Size)

// Función para inicializar la memoria.
void m_bnb_init(int argc, char **argv) {
  if (memory_v != NULL) {
    free(memory_v);      // Libera la memoria previamente asignada.
    memory_v = NULL;
  }
  if (process_addr != NULL) {
    free(process_addr);  // Libera las direcciones de procesos previamente asignadas.
    process_addr = NULL;
  }

  size_t cant_block = Kb(m_size());  // Calcula la cantidad de bloques necesarios.
  memory_v = (Block *)malloc(sizeof(Block) * cant_block);  // Asigna memoria para los bloques.
  process_addr = (size_t *)malloc(sizeof(size_t) * cant_block);  // Asigna memoria para las direcciones de procesos.
  actual_addr = 0;

  FreeBlock* initial_block = (FreeBlock*)malloc(sizeof(FreeBlock));
  initial_block->start_addr = 0;
  initial_block->end_addr = cant_block * Block_Size - 1;
  initial_block->next = NULL;
  free_blocks = initial_block;

  for (size_t i = 0, start = 0; i < cant_block; i++, start += Block_Size) {
    Block *currentBlock = &memory_v[i];
    currentBlock->heap = start + Code_Size;  // Inicializa la dirección de heap del bloque.
    currentBlock->stack = start + Block_Size - 1;  // Inicializa la dirección de stack del bloque.
    currentBlock->s_addr = start + Code_Size;  // Inicializa la dirección de inicio del bloque.
    currentBlock->e_addr = start + Block_Size - 1;  // Inicializa la dirección de fin del bloque.
    currentBlock->size = 0;  // Inicializa el tamaño del bloque.
    currentBlock->in_use = 0;  // Indica si el bloque está en uso.
    currentBlock->owner = NO_ONWER;  // Indica el propietario del bloque.
  }
}

// Función para asignar memoria (malloc).
int m_bnb_malloc(size_t size, ptr_t *out) {
  for (size_t i = 0; i < Kb(m_size()); i++) {
    if (!memory_v[i].in_use) {  // Encuentra un bloque no utilizado.
      m_set_owner(i * Block_Size + Code_Size, i * Block_Size + Block_Size - 1);  // Establece el propietario.
      process_addr[actual_Pid] = i;  // Asigna la dirección del proceso.
      actual_addr = i;  // Actualiza la dirección actual.
      memory_v[i].in_use = 1;  // Marca el bloque como en uso.
      memory_v[i].owner = actual_Pid;  // Asigna el propietario.
      memory_v[i].size = size;  // Establece el tamaño del bloque.
      out->addr = i * Block_Size + Code_Size;  // Asigna la dirección de inicio del bloque a la salida.
      out->size = 1;  // Tamaño de 1 bloque.

      // Actualiza la lista de bloques libres:
      FreeBlock* current = free_blocks;
      FreeBlock* prev = NULL;

      while (current) {
        if (current->start_addr == i * Block_Size + Code_Size) {
          if (current->start_addr + current->end_addr == i * Block_Size + Block_Size - 1) {
            // El bloque libre coincide exactamente con el bloque asignado, eliminarlo.
            if (prev) {
              prev->next = current->next;
            } else {
              free_blocks = current->next;
            }
            free(current);
          } else {
            // El bloque libre es parte del bloque asignado, ajustar su dirección de inicio.
            current->start_addr = i * Block_Size + Code_Size + size;
          }
          break;
        }
        prev = current;
        current = current->next;
      }

      return 0;  // Éxito.
    }
  }
  return 1;  // Fallo al asignar memoria.
}

// Función para liberar memoria (free).
int m_bnb_free(ptr_t ptr) {
  size_t start = memory_v[actual_addr].s_addr;
  size_t end = memory_v[actual_addr].e_addr;

  if (ptr.addr >= start && ptr.addr + ptr.size < end) {  // Comprueba si la dirección pertenece al bloque actual.
   // m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);  // Libera el propietario de las direcciones.
    memory_v[actual_addr].size -= ptr.size;  // Actualiza el tamaño del bloque.

    // Agregar el bloque liberado a la lista de bloques libres:
    FreeBlock* new_free_block = (FreeBlock*)malloc(sizeof(FreeBlock));
    new_free_block->start_addr = ptr.addr;
    new_free_block->end_addr = ptr.addr + ptr.size - 1;
    new_free_block->next = free_blocks;
    free_blocks = new_free_block;

    return 0;  // Éxito.
  }

  return 1;  // Fallo al liberar memoria.
}

// Función para empujar un valor en la pila.
int m_bnb_push(byte val, ptr_t *out) {
  if (memory_v[actual_addr].stack - 1 <= memory_v[actual_addr].heap) {
    return 1;  // Fallo, la pila está llena.
  }

  m_write(memory_v[actual_addr].stack, val);  // Escribe el valor en la pila.
  memory_v[actual_addr].stack--;  // Actualiza la posición de la pila.
  out->addr = memory_v[actual_addr].stack;  // Asigna la dirección de la pila a la salida.
  return 0;  // Éxito.
}

// Función para sacar un valor de la pila.
int m_bnb_pop(byte *out) {
  size_t stack_top = memory_v[actual_addr].stack + 1;
  size_t start_block = memory_v[actual_addr].s_addr;
  size_t end_block = memory_v[actual_addr].e_addr;

  if (stack_top >= start_block + end_block) {
    return 1;  // Fallo, no hay elementos en la pila.
  }

  *out = m_read(stack_top);  // Lee el valor en la cima de la pila.
  memory_v[actual_addr].stack++;  // Actualiza la posición de la pila.
  return 0;  // Éxito.
}

// Función para cargar un byte desde una dirección.
int m_bnb_load(addr_t addr, byte *out) {
  size_t start = memory_v[actual_addr].s_addr;
  size_t end = memory_v[actual_addr].e_addr;

  if (addr >= start && addr < end) {  // Comprueba si la dirección pertenece al bloque actual.
    *out = m_read(addr);  // Lee el byte desde la dirección especificada.
    return 0;  // Éxito.
  }

  return 1;  // Fallo al cargar el byte.
}

// Función para almacenar un byte en una dirección.
int m_bnb_store(addr_t address, byte value) {
  size_t startAddress = memory_v[actual_addr].s_addr;
  size_t currentSize = memory_v[actual_addr].size;

  if (address >= startAddress && address < startAddress + currentSize) {  // Comprueba si la dirección pertenece al bloque actual.
    m_write(address, value);  // Escribe el byte en la dirección especificada.
    return 0;  // Éxito.
  }

  return 1;  // Fallo al almacenar el byte.
}

// Función para manejar el cambio de contexto de un proceso.
void m_bnb_on_ctx_switch(process_t process) {
  actual_Pid = process.pid;  // Actualiza el ID del proceso actual.
  actual_addr = process_addr[process.pid];  // Actualiza la dirección actual.
}

// Función para manejar el final de un proceso.
void m_bnb_on_end_process(process_t process) {
  size_t addr = process_addr[process.pid];  // Obtiene la dirección del proceso.
  m_unset_owner(memory_v[addr].s_addr, memory_v[addr].e_addr);  // Libera el propietario de las direcciones del bloque.
  memory_v[addr].in_use = 0;  // Marca el bloque como no utilizado.
  memory_v[addr].owner = NO_ONWER;  // Establece el propietario como NO_ONWER.
  memory_v[addr].size = 0;  // Establece el tamaño del bloque como 0.
  memory_v[addr].heap = memory_v[addr].s_addr;  // Reinicia la dirección de heap.
  memory_v[addr].stack = memory_v[addr].e_addr;  // Reinicia la dirección de stack.
}
