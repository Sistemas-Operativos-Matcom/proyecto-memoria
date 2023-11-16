#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"
#include "free_list.h"

typedef struct bnb_virtual_process
{
  int pid;
  int base, bound;
  int stack_ptr;
  int heap_ptr;
  free_list_t heap;
} bnb_virtual_process_t;
#define MAX_PROC_COUNT 5

static bnb_virtual_process_t *virtual_memory;
static unsigned char is_init = 0;
static int last_assigned = -1;         // última posición en virtual_memory que tiene un proceso
static size_t virtual_memory_size = 0; // tamano de la memoria virtual
static int curr_process_pos = -1;      // posición en el virtual_memory del proceso actual

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{

  // Limpiar virtual_memory si fue asignada
  if (is_init)
    free(virtual_memory);

  // A cada proceso le asigno una memoria virtual de igual tamaño
  virtual_memory_size = m_size() / MAX_PROC_COUNT;
  virtual_memory = malloc(MAX_PROC_COUNT * sizeof(bnb_virtual_process_t));

  // Como al principio no hay procesos, su pid es -1
  for (int i = 0; i < MAX_PROC_COUNT; i++)
  {
    virtual_memory[i].pid = -1;
  }

  // Como no hay una última posición asignada
  last_assigned = -1;
  curr_process_pos = -1;
  is_init = 1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  free_list_t *heap = &virtual_memory[curr_process_pos].heap;
  size_t addr = -1;

  // Obtiene el primer espacio vacío en el heap para reservarlo
  int status = Get_from_memory(heap, size, &addr);

  out->addr = (size_t)addr;
  out->size = size;

  return status;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  free_list_t *heap = &virtual_memory[curr_process_pos].heap;

  // En caso de estar ocupado el espacio en el heap, lo libera
  int status = Get_from_memory(heap, ptr.size, ptr.addr);

  return status;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  size_t stack_ptr = virtual_memory[curr_process_pos].stack_ptr;
  size_t heap_ptr = virtual_memory[curr_process_pos].heap_ptr;

  // No es posible hacer push si el stack está lleno
  if (heap_ptr >= stack_ptr - 1)
  {
    return MEM_FAIL;
  }

  // Al pushear elementos, el stack pointer disminuye
  stack_ptr--;
  size_t p_stack_ptr = virtual_memory[curr_process_pos].base + stack_ptr;
  virtual_memory[curr_process_pos].stack_ptr--;

  // Guardar el valor en la memoria física
  m_write(p_stack_ptr, val);

  out->addr = stack_ptr;
  out->size = 1;

  return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  int stack_ptr = virtual_memory[curr_process_pos].stack_ptr;

  // Si el stack pointer se pasa del tamaño máximo no hay ningún elemento al que hacerle pop
  if (stack_ptr + 1 > (int)virtual_memory_size)
  {
    return MEM_FAIL;
  }

  // Obtiene la dirección física
  int addr = stack_ptr + virtual_memory[curr_process_pos].base;

  *out = m_read(addr);
  // Al hacer pop, el stack pointer aumenta
  virtual_memory[curr_process_pos].stack_ptr++;

  return MEM_SUCCESS;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  // La dirección es mayor que bound
  if (addr >= (addr_t)virtual_memory[curr_process_pos].bound)
    return MEM_FAIL;

  // Obtener la direccion fisica de la cual se quiere cargar el valor
  int p_addr = (int)addr + virtual_memory[curr_process_pos].base;

  // Guardar el valor cargado en out
  *out = m_read(p_addr);

  return MEM_SUCCESS;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  // Si la direccion es mayor que bound
  if (addr >= (addr_t)virtual_memory[curr_process_pos].heap_ptr)
    return MEM_FAIL;

  free_list_t heap = virtual_memory[curr_process_pos].heap;
  node_t *node = heap.top;
  size_t prev_last_page_frame = node->first_page_frame + node->num_pages;
  size_t next_first_page_frame = node->next != NULL ? node->next->first_page_frame : heap.max_page_frame;
  while (prev_last_page_frame <= (size_t)addr && addr + 1 <= next_first_page_frame)
  {
    if ((prev_last_page_frame <= addr && addr + 1 <= next_first_page_frame) || addr + 1 <= node->first_page_frame)
    {
      // Obtener la direccion fisica  en la que se desea guardar el valor
      int p_addr = (int)addr + virtual_memory[curr_process_pos].base;
      m_write(p_addr, val);
      return MEM_SUCCESS;
    }
    node = node->next;
  }
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  int empty_space = -1;
  // Buscar el proceso
  for (int i = 0; i <= last_assigned; i++)
  {
    // Si se encuentra se retorna
    if (virtual_memory[i].pid == process.pid)
    {
      curr_process_pos = i;
      return;
    }
    // Se guarda el primer espacio vacío
    else if (virtual_memory[i].pid == -1 && empty_space == -1)
    {
      empty_space = i;
    }
  }

  int pos = empty_space;

  // Si hay un espacio vacio, se guarda el proceso en ese espacio
  // Si no existe, se le asigna un espacio nuevo a ese proceso
  if (empty_space == -1)
  {
    last_assigned++;
    pos = last_assigned;

    // Inicializar el heap
    Init_free_list(&virtual_memory[pos].heap, virtual_memory_size / 2);
  }
  else
  {
    // Eliminar los valores en el heap del proceso anterior
    Reset_free_list(&virtual_memory[pos].heap, virtual_memory_size / 2);
  }

  // Actualizar los valores
  virtual_memory[pos].pid = process.pid;
  virtual_memory[pos].base = last_assigned * virtual_memory_size;
  virtual_memory[pos].bound = virtual_memory_size;
  virtual_memory[pos].stack_ptr = virtual_memory_size;
  virtual_memory[pos].heap_ptr = virtual_memory_size / 2;

  // Guardar el proceso como proceso actual
  curr_process_pos = pos;

  m_set_owner(virtual_memory[pos].base, virtual_memory[pos].base + virtual_memory[pos].bound);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  // Buscamos el proceso en virtual_memory y y le asignamos pid -1, ya que terminó
  for (int i = 0; i <= last_assigned; i++)
  {
    if (virtual_memory[i].pid == process.pid)
    {
      m_unset_owner(virtual_memory[i].base, virtual_memory[i].base + virtual_memory[i].bound);
      virtual_memory[i].pid = -1;
      return;
    }
  }
}
