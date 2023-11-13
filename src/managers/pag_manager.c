#include "pag_manager.h"
#include "../memory.h"
#include "../tests.h"
#include "stdio.h"
#include "pag_stack.h"

int* memory;
process_t p_current_process;
int p_current_index = 0;
process_t *p_processes;
Pag_Stack_t *p_process_stack;
Pag_Heap_t *p_process_heap;
int p_process_len = 0;
int p_default_size = 512;


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  memory = (int*)malloc(m_size()*sizeof(int*));
  p_processes = (process_t*)malloc(MAX_PROGRAM_COUNT * sizeof(process_t));
  for(int i = 0; i < MAX_PROGRAM_COUNT; i++)
  {
    p_processes[i].pid = -1;
  }
  p_process_stack = (Pag_Stack_t*)malloc(MAX_PROGRAM_COUNT * sizeof(Pag_Stack_t));
  p_process_heap = (Pag_Heap_t*)malloc(MAX_PROGRAM_COUNT * sizeof(Pag_Heap_t));
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  Pag_Heap_t *heap = &p_process_heap[p_current_index];

  // If there is space to reserve, allocate space and return pointer
  int allocate = heap->pag_reserve(size, heap);
  if(allocate != -1)
  {
    out->addr = (size_t)allocate;
    out->size = size;
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented free\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  Pag_Stack_t *stack = &p_process_stack[p_current_index];
  return stack->pag_push(&val, out, stack);
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  Pag_Stack_t *stack = &p_process_stack[p_current_index];
  return stack->pag_pop(out, stack);
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  Pag_Heap_t *heap = &p_process_heap[p_current_index];
  return heap->pag_load(addr, out, heap);
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  Pag_Heap_t *heap = &p_process_heap[p_current_index];
  return heap->pag_store(addr, val, heap);
}

int find_free_space(size_t size)
{
  for(int i = 0; i < (int)m_size(); i++)
  {
    if(memory[i] == 0)
    {
      for(int j = i; j < (int)m_size(); j++)
      {
        if(j-i == (int)size)
        {
          for(int m = i; m < j; m++)
          {
            memory[m] = 1;
          }
          return i;
        } 
      }
    }    
  }
  return -1;
}

void p_set_next_process_memory(process_t process)
{
  //Setting processes array 
  p_processes[p_process_len] = process;

  // Finding free space to set process stack
  int size = 512;
  int base;
  for(int i = size; i > 0; i--)
  {
    base = find_free_space(i);
    if(base != -1)
    {
      size = i;
      break;
    } 
  }

  if(base == -1) exit(1); //Not enought space in memory

  int bound = base + size;
  
  int stack_base = base + process.program->size;
  Pag_Stack_t stack = Pag_Stack_init(stack_base, size);
  p_process_stack[p_process_len] = stack;

  // Finding free space to set process heap
  size = 512;
  int heap_start;
  for(int i = size; i > 0; i--)
  {
    heap_start = find_free_space(i);
    if(heap_start != -1)
    {
      size = i;
      break;
    } 
  }

  Pag_Heap_t heap = Pag_Heap_init(heap_start, size);
  p_process_heap[p_process_len] = heap;
  
  // Setting owner in space used to code-stack and heap
  m_set_owner(base, bound);
  m_set_owner(heap_start, heap_start+size);


  //Update pointer to next process' position and index for current process
  p_current_index = p_process_len;
  p_process_len++;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  for(int i = 0; i < p_process_len; i++)
  {
    if(process.pid == p_processes[i].pid)
    {
      p_current_index = i;
      p_current_process = process;
      return;
    }
  }
  p_set_next_process_memory(process);
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  
  if(p_current_process.pid == process.pid) p_current_process.pid = -1;

  //Freeing stack pages
  Pag_Stack_t *stack = &p_process_stack[p_current_index];
  for (int i = 0; i < 5; i++)
  {
    if(stack->SP[i] != -1)
    {
      m_unset_owner((addr_t)stack->pages_base[i], (addr_t)stack->pages_bound[i]);
    }
  }

  // Freeing heap pages
  Pag_Heap_t *heap = &p_process_heap[p_current_index];
  for (int i = 0; i < 5; i++)
  {
    if(heap->len[i] != -1)
    {
      m_unset_owner((addr_t)heap->pages_base[i], (addr_t)heap->pages_bound[i]);
    }
  }

}
