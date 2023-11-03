#include "pag_manager.h"

#include "stdio.h"
#include "stack.h"
#include "process_pag.h"
#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include "../tests.h"
#include "../memory.c"
#include "process_list.h"
#define PAGE_SIZE 128

process_List_t *list_of_process;
sizeList_t *free_page_frames;
int is_list_of_process_created = 0;
process_pag_t *current;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  if (is_list_of_process_created)
  {
    list_of_process = p_init();
    is_list_of_process_created = 1;
  }
  p_reset(list_of_process);

  sizeList_t *free_page_frames = init();
  free_page_frames->size = m_size() / PAGE_SIZE;
  for (size_t i = 0; i < free_page_frames->size; i++)
  {
    *(free_page_frames->data + i) = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  // buscando espacio para almacenar una cantidad "size" de  bytes
  size_t counter_free_spaces_consecutive = 0;
  for (size_t i = 0; i < current->v_memory->heap->len; i++)
  {
    if (counter_free_spaces_consecutive == size)
    {
      for (size_t j = 0; j < size; j++)
      {
        set(current->v_memory->heap, i - j, 1);
      }
      out->addr = (i - size - 1) + current->process.program->size;
      return 0;
    }
    if (get(current->v_memory->heap, i) == 0)
    {
      counter_free_spaces_consecutive++;
    }
    else
    {
      counter_free_spaces_consecutive = 0;
    }
  }

  // si no hay espacio suficiente para almacenar "size" bytes
  // crear nuevo espacio
  if (current->v_memory->heap->len + size >= current->v_memory->heap->size)
  {
    increaseSize(current->v_memory->heap);
    // if size>=128 then
    // paginar este nuevo espacio en pages with pages frame como hice en otro metodo

    for (size_t i = 0; i < size; i++)
    {
      push(current->v_memory->heap, 0);
    }
    return 0;
  }
  else
  {
    // if size>=128 then
    // paginar este nuevo espacio en pages with pages frame como hice en otro metodo

    for (size_t i = 0; i < size; i++)
    {
      push(current->v_memory->heap, 0);
    }
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  set(current->v_memory->heap, ptr.addr, 0);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  if (current->v_memory->stack->sp + 1 <= current->v_memory->stack->size_reserve)
    current->v_memory->stack->sp++;
  else
    return 1;
  // revisar si la page frame donde va el sp esta libre o pertenece a dicho proceso o offset asi
  m_write(current->v_memory->stack->sp + offset, val);
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (current->v_memory->stack->sp - 1 >= 0)
    current->v_memory->stack->sp--;
  else
    return 1;
  // revisar si la page frame donde va el sp esta libre o pertenece a dicho proceso o offset asi
  *out = m_read(current->v_memory->stack->sp + offset); // puede ser q deje un page frame vacio sin embargo el proceso siga siendo owner de dicho page frame,si no es uno de los page frame por default del proceso,puedo eliminarlo,es decir dejarlo libre
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  *out = m_read(addr + offset);
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{

  // if (addr + offset > current->v_memory->heap->size)
  // {
  //   increaseSize(current->v_memory->heap);
  //   for (size_t i = 0; i < PAGE_SIZE; i++)
  //   {
  //     push(current->v_memory->heap, 0);
  //   }

  // poniendo en ocupado cierta pagina de current_process
  for (size_t i = 0; i < free_page_frames->len; i++)
  {
    if (get(free_page_frames, i) == 0)
    {
      set(free_page_frames, i, 1);
      push(current->page_frames_indexed_by_virtual_pages, i); // add nueva pagina a la tabla de pagina de current process
      break;
    }
  }

  m_write(addr + offset, val);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  // agregar el proceso a la lista de procesos si es un proceso nuevo
  int is_new_process = 1;
  for (size_t i = 0; i < list_of_process->len; i++)
  {
    if (p_get(list_of_process, i)->process.pid == process.pid)
    {
      is_new_process = 0;
    }
  }
  if (is_new_process)
  {
    // inicializar las cosas del process_pag este
    process_pag_t *new_process = init_process_pag(process);
    p_push(list_of_process, new_process);
  }
  // sustituir
  if (list_of_process->len > 0)
  {
    for (size_t i = 0; i < length(list_of_process); i++)
    {
      if (p_get(list_of_process, i)->process.pid == process.pid)
      {
        current = p_get(list_of_process, i);
      }
    }
  }
  else
  {
    printf("error en cambio de contexto");
    exit(1);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  p_deleteAt(list_of_process, current);
}
