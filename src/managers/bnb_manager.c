#include <malloc.h>
#include "bnb_manager.h"
#include "stdio.h"
#include "../freelist.h"

#define bool int
#define true 1
#define false 0
#define bound 512

typedef struct memory_slot
{
  size_t base; // No eliminar este campo
  size_t heap_base;
  size_t stack_pointer;
  bool used;
  int process_pid;
  free_list *heap;
} memory_slot;

static memory_slot *mem_slots;
static int memory_slots_count;
static int actual_ctx;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  memory_slots_count = m_size() / bound;
  mem_slots = malloc(memory_slots_count * sizeof(memory_slot));
}
// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{

  int result = memory_malloc(mem_slots[actual_ctx].heap, size, out);
  out->addr = out->addr + mem_slots[actual_ctx].heap_base;
  return !result;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  ptr.addr = ptr.addr - mem_slots[actual_ctx].heap_base;
  return !memory_free(mem_slots[actual_ctx].heap, ptr);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (memory_reduce(mem_slots[actual_ctx].heap))
  {
    mem_slots[actual_ctx].stack_pointer = mem_slots[actual_ctx].stack_pointer - 1;
    m_write(mem_slots[actual_ctx].stack_pointer, val);
    out->addr = mem_slots[actual_ctx].stack_pointer - mem_slots[actual_ctx].base;
    out->size = 1;

    return 0;
  }
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{

  if (mem_slots[actual_ctx].stack_pointer < mem_slots[actual_ctx].base + bound)
  {
    *out = m_read(mem_slots[actual_ctx].stack_pointer);
    mem_slots[actual_ctx].stack_pointer = mem_slots[actual_ctx].stack_pointer + 1;
    memory_expand(mem_slots[actual_ctx].heap);
    return 0;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  *out = m_read(mem_slots[actual_ctx].base + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  m_write(mem_slots[actual_ctx].base + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  set_curr_owner(process.pid);
  int free_slot_index = -1;
  for (int i = 0; i < memory_slots_count; i++)
  {
    if (mem_slots[i].used && mem_slots[i].process_pid == process.pid)
    {
      actual_ctx = i;
      return;
    }
    if (free_slot_index == -1 && !mem_slots[i].used)
      free_slot_index = i;
  }
  // It's a new process
  if (free_slot_index == -1)
  {
    fprintf(stderr, "There is't espace in RAM for another process\n");
    exit(1);
  }
  else
  {
    mem_slots[free_slot_index].used = true;
    mem_slots[free_slot_index].process_pid = process.pid;
    mem_slots[free_slot_index].heap_base = bound * free_slot_index + process.program->size;
    mem_slots[free_slot_index].stack_pointer = bound * (free_slot_index + 1);
    mem_slots[free_slot_index].base = bound * free_slot_index;
    mem_slots[free_slot_index].heap = new_free_list(bound - process.program->size);
    m_set_owner(mem_slots[free_slot_index].base, mem_slots[free_slot_index].stack_pointer-1);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{

  int proces_memory_slots = -1;
  for (int i = 0; i < memory_slots_count; i++)
  {
    if (mem_slots[i].process_pid == process.pid)
    {
      proces_memory_slots = i;
    }
  }
  if (proces_memory_slots == -1)
  {
    fprintf(stderr, "Process doesn't exist\n");
    exit(1);
  }
  else
  {
    mem_slots[proces_memory_slots].used = false;
    m_unset_owner(mem_slots[proces_memory_slots].base, mem_slots[proces_memory_slots].stack_pointer-1);
  }
}
