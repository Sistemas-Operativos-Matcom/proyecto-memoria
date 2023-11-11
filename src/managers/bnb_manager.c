#include "bnb_manager.h"

#include "linked_list.h"
#include "stdio.h"

int max_amount_of_process = 32; // esto debemos cambiarlo dependeria del so
int bound;
int *free_list; // true si esta free
int *active_pid;

process_t current_process; // proceso del contexto actual

addr_t *base_by_pid;
addr_t *stackPointer_by_pid;
linked_list_t **heap_free_list_by_pid;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  base_by_pid = (addr_t *)malloc(sizeof(addr_t) * max_amount_of_process);
  stackPointer_by_pid = (addr_t *)malloc(sizeof(addr_t) * max_amount_of_process);
  free_list = (int *)malloc(sizeof(int) * max_amount_of_process);
  active_pid = (int *)malloc(sizeof(int) * max_amount_of_process);
  heap_free_list_by_pid = (linked_list_t **)malloc(sizeof(linked_list_t *) * max_amount_of_process);

  bound = m_size() / max_amount_of_process;
  
  for (int i = 0; i < max_amount_of_process; i++)
  {
    base_by_pid[i] = -1;
    free_list[i] = 1;
    active_pid[i] = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  linked_list_t *free_space = find_fit(heap_free_list_by_pid[current_process.pid], size);

  if (free_space == NULL)
    return 1; // sino hay epacio libre no se pude reservar por tanto da error

  // generar el puntero out
  out->addr = free_space->start.addr;
  out->size = size;

  update(free_space, free_space->start.size - size); // actulizar el espacio libre

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  if (validate_ptr(ptr))
  {
    insert(heap_free_list_by_pid[current_process.pid], ptr);
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  // agregar a memoria q ni idea
  stackPointer_by_pid[current_process.pid]--; // aumentar el stackpointer

  // stackoverflow
  if (stackPointer_by_pid[current_process.pid] < base_by_pid[current_process.pid] + bound / 2)
  {
    return 1;
  }

  m_write(stackPointer_by_pid[current_process.pid], val);

  // verificar que no hubo fallo al aumentar el stackpointer
  out->addr = stackPointer_by_pid[current_process.pid];
  out->size = sizeof(byte);

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  // verificar que el stack no esta vacio
  if (stackPointer_by_pid[current_process.pid] >= base_by_pid[current_process.pid] + bound)
    return 1;

  // devolver memoria
  *out = m_read(stackPointer_by_pid[current_process.pid]);

  stackPointer_by_pid[current_process.pid]++; // disminuir el stackpointer

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if (validate_addr(addr))
  {
    *out = m_read(addr); 

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (validate_addr(addr))
  {
    m_write(addr, val);
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  current_process = process;

  if (!active_pid[process.pid])
  { // inicializar el proceso por primera vez
    for (int i = 0; i < max_amount_of_process; i++)
    {
      if (free_list[i])
      {
        base_by_pid[process.pid] = i * bound;

        stackPointer_by_pid[process.pid] = (addr_t)((i + 1) * bound);
        free_list[i] = 0;

        m_set_owner(base_by_pid[process.pid], base_by_pid[process.pid] + bound - 1);
        break;
      }
    }

    // al inicializar el proceso e heap es un solo segmento con todo el tamaño del heap
    ptr_t _ptr;
    _ptr.addr = base_by_pid[process.pid];
    _ptr.size = bound / 2;
    
    heap_free_list_by_pid[process.pid] = (linked_list_t *)malloc(sizeof(linked_list_t));
    heap_free_list_by_pid[process.pid]->prev = NULL;
    heap_free_list_by_pid[process.pid]->next = NULL;
    heap_free_list_by_pid[process.pid]->start = _ptr;

    active_pid[process.pid] = 1;

  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  active_pid[process.pid] = 0;
  free_list[process.pid] = 1;
  m_unset_owner(base_by_pid[process.pid], base_by_pid[process.pid] + bound - 1);
  base_by_pid[process.pid] = -1; 
  free_entire_list(heap_free_list_by_pid[process.pid]); 
  stackPointer_by_pid[process.pid] = -1;
}

int validate_ptr(ptr_t ptr)
{
  if (ptr.addr >= base_by_pid[current_process.pid] && ptr.addr + ptr.size < base_by_pid[current_process.pid] + (bound / 2) && is_valid_ptr(heap_free_list_by_pid[current_process.pid], ptr))
    return 1;

  return 0;
}

int validate_addr(addr_t addr)
{
  if (addr >= base_by_pid[current_process.pid] && addr < base_by_pid[current_process.pid] + (bound / 2) && !is_free(heap_free_list_by_pid[current_process.pid], addr))
    return 1;

  return 0;
}