#include "bnb_manager.h"
#include "../memory.h"
#include "../tests.h"
#include "stdio.h"
#include "stack.h"
#include "heap.h"

size_t mem_size;
process_t current_process;
int current_index = 0;
process_t *processes;
int *process_bound;
Stack_t *process_stack;
Heap_t *process_heap;
int process_len = 0;
int default_size = 512;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  processes = (process_t*)malloc(MAX_PROGRAM_COUNT * sizeof(process_t));
  process_bound = (int*)malloc(MAX_PROGRAM_COUNT * sizeof(int));
  process_stack = (Stack_t*)malloc(MAX_PROGRAM_COUNT * sizeof(Stack_t));
  process_heap = (Heap_t*)malloc(MAX_PROGRAM_COUNT * sizeof(Heap_t));
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  Stack_t *stack = &process_stack[current_index];
  Heap_t *heap = &process_heap[current_index];

  // If there is space to reserve, allocate space and return pointer
  int allocate = heap->reserve(size, stack->top, heap);
  if(allocate != -1)
  {
    out->addr = (size_t)allocate;
    out->size = size;
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  Stack_t stack = process_stack[current_index];
  Heap_t heap = process_heap[current_index];

  return stack.push(&val, out, &stack, heap.from_addr);
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  Heap_t *current_heap = &process_heap[current_index];

  //Can't add out of heap
  if(addr > current_heap->to_addr || addr < current_heap->from_addr) return 1;
  
  // Busy or not allocated address
  if(current_heap->used_slots[addr] == 1 || current_heap->used_slots[addr] == 0) return 1;

  // Write val in memory
  m_write(addr, val);
  current_heap->used_slots[addr] = 1;
  return 0;
}

void set_next_process_memory(process_t process)
{
  //Setting curr_procss 
  current_process = process;
  processes[process_len] = process;

  //Setting base, bound, stack and heap of new process
  int base = process_bound[process_len-1] + 1;
  if(process_len == 0) base = 0; //First slot is different

  int bound = base + default_size;
  process_bound[process_len] = bound;
  
  int stack_base = base + process.program->size;

  Stack_t stack = Stack_init(stack_base);
  process_stack[process_len] = stack;

  Heap_t heap = Heap_init((addr_t)bound, default_size-process.program->size-1);
  process_heap[process_len] = heap;
  
  // Setting owner in process' memory
  m_set_owner(base, bound);

  //Update pointer to next process' position and index for current process
  current_index = process_len;
  process_len++;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  for(int i = 0; i < process_len; i++)
  {
    if(process.pid == processes[i].pid)
    {
      current_index = i;
      current_process = process;
      return;
    }
  }
  set_next_process_memory(process);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
