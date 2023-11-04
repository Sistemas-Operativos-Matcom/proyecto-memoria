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
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
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
  fprintf(stderr, "Not Implemented\n");
  exit(1);
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
  m_set_owner(base, bound);
  int stack_base = base + process.program->size;
  process_heap[process_len] = Heap_init(bound);
  process_stack[process_len] = Stack_init(stack_base, process_heap[process_len]);

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
      fprintf(stderr, "changed to: %d", process.pid);
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
