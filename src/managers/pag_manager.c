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
size_t amount_new_elements_in_heap_before_add_new_v_page = 0;
int is_first_time_running = 1;
process_pag_t *current;

addr_t convert_va_in_pa(addr_t va) // checked, analizar problema con -1,lo demas esta perfecto
{
  size_t vpn = get_vpn_of_va(va);
  return (get(current->page_frames_indexed_by_virtual_pages, vpn) - 1) * PAGE_SIZE + (va % PAGE_SIZE); // posible error con el -1 pq la primera pagina es la 0,no la 1, entonces cuando es 7*PAGE_SIZE, quisas era 6*PAGE_SIZE
}

size_t get_vpn_of_va(addr_t va) // checcked
{
  size_t vpn = 0;
  // getting virtual page number of va addr in current.page_frames_indexed_by_virtual_pages
  for (size_t i = 1; i <= va; i++)
  {
    if (i % PAGE_SIZE == 0)
      vpn++;
  }
  return vpn;
}
void Add_Free_Page_Frame_to_page_frames_indexed_by_v_pages(sizeList_t *free_page_frames, sizeList_t *page_frames_indexed_by_virtual_pages, size_t amount_of_new_pages_to_add) // checked,no big errors found
{
  size_t counter = 0;
  while (counter < amount_of_new_pages_to_add)
  {
    size_t first_free_page_frame_founded = 0;
    for (size_t i = 0; i < free_page_frames->len; i++)
    {
      if (get(free_page_frames, i) == 0)
      {
        first_free_page_frame_founded = i;
        break;
      }
    }
    push(page_frames_indexed_by_virtual_pages, first_free_page_frame_founded);
  }
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) // checked,no big errors found
{
  if (is_first_time_running)
  {
    list_of_process = p_init();
    is_first_time_running = 0;
    free_page_frames = init();
    current = (process_pag_t *)malloc(sizeof(process_pag_t));
  }
  else
  {
    p_reset(list_of_process);
    reset(free_page_frames);
  }

  free_page_frames->size = m_size() / PAGE_SIZE;
  for (size_t i = 0; i < free_page_frames->size; i++)
  {
    *(free_page_frames->data + i) = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) // checked,no estoy ocupando el espacio q reservo en el heap, en la free_page_frames, debo marcar esas direcciones q reserve como ocupado para poder liberarlas despues ,sino el metodo de liberar no tiene sentido
{
  // buscando espacio para almacenar una cantidad "size" de  bytes
  // y devuelve el puntero sumado con el size del codigo + el size del stack
  size_t counter_free_spaces_consecutive = 0;
  for (size_t i = 0; i < current->v_memory->heap->list->len; i++)
  {
    if (counter_free_spaces_consecutive == size)
    {
      for (size_t j = 0; j < size; j++)
      {
        set(current->v_memory->heap, i - j, 1);
      }
      out->addr = (i - size - 1) + current->process.program->size + current->v_memory->stack->size_reserve;
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
  while (1)
  {
    if (current->v_memory->heap->list->len + size >= current->v_memory->heap->list->size) // OOOOJJJJJJOOOOO  analizar si heap y paginas virtuales estan sincronizadas y crecen por igual
    {
      current->v_memory->heap->list->size += PAGE_SIZE;
      Add_Free_Page_Frame_to_page_frames_indexed_by_v_pages(free_page_frames, current->page_frames_indexed_by_virtual_pages);
    }
    else
      break;
  }
  // ocupar una cantidad size en la lista
  for (size_t i = 0; i < size; i++)
  {
    push(current->v_memory->heap, 0);
    amount_new_elements_in_heap_before_add_new_v_page++;
  }

  out->addr = current->v_memory->heap->list->len - 1 - size + current->process.program->size + current->v_memory->stack->size_reserve;

  if (amount_new_elements_in_heap_before_add_new_v_page > 128)
  {
    size_t amount_of_new_pages_to_add = (amount_new_elements_in_heap_before_add_new_v_page % PAGE_SIZE) == 0 ? amount_new_elements_in_heap_before_add_new_v_page / PAGE_SIZE : (amount_new_elements_in_heap_before_add_new_v_page / PAGE_SIZE) + 1;
    amount_new_elements_in_heap_before_add_new_v_page = 0;
    Add_Free_Page_Frame_to_page_frames_indexed_by_v_pages(free_page_frames, current->page_frames_indexed_by_virtual_pages, amount_of_new_pages_to_add);
  }

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  // vpn of ptr,if ptr is the start pointer of a pageframe and in that pageframe there are no more ptr , then that pageframe should be free
  if (get_vpn_of_va(ptr.addr)//tengo q arreglar malloc primero para poder saber que liberar
    set(current->v_memory->heap, ptr.addr, 0);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) // no estoy liberando ni guardando en la memoria fisica el sp,error,preguntar si necesario
{
  if (current->v_memory->stack->sp + 1 <= current->v_memory->stack->size_reserve)
    current->v_memory->stack->sp++;
  else
    return 1;

  m_write(convert_va_in_pa(current->v_memory->stack->sp), val);
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{ // lo mismo q en el de arriba tengo q hacer
  if (current->v_memory->stack->sp - 1 >= 0)
    current->v_memory->stack->sp--;
  else
    return 1;

  *out = m_read(convert_va_in_pa(current->v_memory->stack->sp));
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  *out = m_read(convert_va_in_pa(addr));
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  // no necesario marcar como ocupado en free_page_frame ya q se marco cuando reserve memoria para el en malloc
  if (addr >= current->v_memory->heap->list->size)
  {
    return 1;
  }

  // for (size_t i = 0; i < PAGE_SIZE; i++)
  // {
  //   push(current->v_memory->heap, 0);
  // }

  // // poniendo en ocupado cierta pagina de current_process
  // for (size_t i = 0; i < free_page_frames->len; i++)
  // {
  //   if (get(free_page_frames, i) == 0)
  //   {
  //     set(free_page_frames, i, 1);
  //     push(current->page_frames_indexed_by_virtual_pages, i); // add nueva pagina a la tabla de pagina de current process
  //     break;
  //   }
  // }

  m_write(convert_va_in_pa(addr), val);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{

  int is_new_process = 1;
  for (size_t i = 0; i < list_of_process->len; i++)
  {
    if (p_get(list_of_process, i)->process.pid == process.pid)
    {
      is_new_process = 0;
    }
  }
  // agregar el proceso a la lista de procesos si es un proceso nuevo
  if (is_new_process)
  {
    process_pag_t *new_process = init_process_pag(process); // veriicar q este sincronizado el tamano del heap con la tabla de pagina
    p_push(list_of_process, new_process);
    current = new_process;
  }
  else
  {
    // sustituir el current por el proceso actual si no es nuevo
    for (size_t i = 0; i < length(list_of_process); i++)
    {
      if (p_get(list_of_process, i)->process.pid == process.pid)
      {
        current = p_get(list_of_process, i);
        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  for (size_t i = 0; i < current->page_frames_indexed_by_virtual_pages->len; i++)
  {
    set(free_page_frames, get(current->page_frames_indexed_by_virtual_pages, i), 0);
  }

  p_deleteAt(list_of_process, current);
}
