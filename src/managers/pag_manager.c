#include "pag_manager.h"
#include <stddef.h>
#include "stdio.h"
#include "stack.h"
#include "process_pag.h"
#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include "../tests.h"
#include "process_list.h"

process_List_t *list_of_process;
sizeList_t *free_page_frames;
// size_t amount_new_elements_in_heap_before_add_new_v_page = 0;
size_t amount_new_elements_in_heap_before_updating_pages_table = 0;
int is_first_time_running = 1;
process_pag_t *current;

size_t get_vpn_of_va(addr_t va) // checcked
{
  size_t vpnn = (size_t)0;
  // getting virtual page number of va addr in current.pages_table
  for (size_t i = 1; i <= va; i++)
  {
    if (i % PAGE_SIZE == (size_t)0)
      vpnn++;
  }
  return vpnn;
}

size_t convert_va_in_pa(addr_t va) // checked, analizar problema con -1,lo demas esta perfecto
{
  size_t vpn = get_vpn_of_va(va);
  return (get(current->pages_table, vpn) - (size_t)1) * (size_t)PAGE_SIZE + ((size_t)va % (size_t)PAGE_SIZE); // posible error con el -1 pq la primera pagina es la 0,no la 1, entonces cuando es 7*PAGE_SIZE, quisas era 6*PAGE_SIZE
}

void Add_Free_Page_Frame_to_pages_table(sizeList_t *free_page_frames, sizeList_t *pages_table, size_t amount_of_new_pages_to_add) // checked,no big errors found
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
    push(pages_table, first_free_page_frame_founded);
    set(free_page_frames, first_free_page_frame_founded, 1);
    m_set_owner(convert_va_in_pa((pages_table->len - 1) * (size_t)PAGE_SIZE), convert_va_in_pa((pages_table->len) * (size_t)PAGE_SIZE));
  }
}

void update_pages_table(process_pag_t *current_process, sizeList_t *free_page_frames_list) // implementar eliminacion de pagina de pages_table
{
  size_t amount_of_new_pages_to_add = 0;

  if (current->v_memory->heap->end_virtual_pointer == current->v_memory->heap->start_virtual_pointer)
  {
    amount_of_new_pages_to_add = (current_process->v_memory->heap->end_virtual_pointer % PAGE_SIZE) == 0 ? current_process->v_memory->heap->end_virtual_pointer / PAGE_SIZE : (current_process->v_memory->heap->end_virtual_pointer / PAGE_SIZE) + 1;
    Add_Free_Page_Frame_to_pages_table(free_page_frames_list, current_process->pages_table, amount_of_new_pages_to_add);
    return;
  }

  if (amount_new_elements_in_heap_before_updating_pages_table > 0)
  {
    for (size_t i = current->v_memory->heap->end_virtual_pointer; i < amount_new_elements_in_heap_before_updating_pages_table + current->v_memory->heap->end_virtual_pointer; i++)
    {
      if (i % PAGE_SIZE == 0)
        amount_of_new_pages_to_add++;
    }
    Add_Free_Page_Frame_to_pages_table(free_page_frames_list, current_process->pages_table, amount_of_new_pages_to_add);
    return;
  }

  // size_t amount_of_pages_to_delete = 0;
  // if (amount_new_elements_in_heap_before_updating_pages_table < 0)
  // {
  //   for (size_t i = current->v_memory->heap->end_virtual_pointer; i < amount_new_elements_in_heap_before_updating_pages_table + current->v_memory->heap->end_virtual_pointer; i++)
  //   {
  //     if (i % PAGE_SIZE == 0)
  //       amount_of_pages_to_delete++;
  //   }
  //   Delete_Free_Page_Frame_of_pages_table(free_page_frames_list, current_process->pages_table, amount_of_pages_to_delete);
  //   return;
  // }
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) // checked
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

  free_page_frames->size = (m_size() % (size_t)PAGE_SIZE) == 0 ? (m_size() / (size_t)PAGE_SIZE) : ((m_size() / (size_t)PAGE_SIZE) + (size_t)1); // creando lista de page frames libres, donde cada elemento es una pagina de 128 bytes de la memoria fisica
  for (size_t i = 0; i < free_page_frames->size; i++)
  {
    push(free_page_frames, 0);
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) // checked
{
  if (current->v_memory->heap->list->len == 0)
  {
    for (size_t i = 0; i < size; i++)
    {
      push(current->v_memory->heap->list, 1);
      amount_new_elements_in_heap_before_updating_pages_table++;
    }
    update_pages_table(current, free_page_frames);               // before updating end_virtual_pointer ya que lo necesito para saber cuantas paginas to add
    amount_new_elements_in_heap_before_updating_pages_table = 0; // reseteando valor global

    current->v_memory->heap->end_virtual_pointer = current->v_memory->heap->list->len + current->v_memory->heap->start_virtual_pointer; // actualizando end_pointer del heap
    out->addr = current->v_memory->heap->end_virtual_pointer;
    return 0;
  }

  // buscando espacio para almacenar una cantidad "size" de  bytes
  // y devuelve el puntero sumado con el size del codigo + el size del stack
  size_t counter_free_spaces_consecutive = 0;
  for (size_t i = 0; i < current->v_memory->heap->list->len; i++)
  {
    if (counter_free_spaces_consecutive == size)
    {
      for (size_t j = 0; j < size; j++)
      {
        set(current->v_memory->heap->list, i - j, 1);
      }
      // dandole lugar en memoria fisica a esta pagina en caso de q en page_table no estuviera mapeada sustituyendo los -1 por nuevas paginas de memoria
      // update_pages_table(current, free_page_frames);
      // si no se hace lo de arriba entonces simplemente ocupo en 1 los lugares esos del heap ya que esta mapeada esta pagina del heap  en memoria fisica
      out->addr = (i - size - 1) + current->v_memory->heap->start_virtual_pointer;
      return 0;
    }
    if (get(current->v_memory->heap->list, i) == 0)
    {
      counter_free_spaces_consecutive++;
    }
    else
    {
      counter_free_spaces_consecutive = 0;
    }
  }

  // si no hay espacio suficiente consecutivo para almacenar "size" bytes
  // crear nuevo espacio al final del heap
  for (size_t i = 0; i < size; i++)
  {
    push(current->v_memory->heap->list, 1);
    amount_new_elements_in_heap_before_updating_pages_table++;
  }
  update_pages_table(current, free_page_frames);               // before updating end_virtual_pointer ya que lo necesito para saber cuantas paginas to add
  amount_new_elements_in_heap_before_updating_pages_table = 0; // reseteando valor global

  current->v_memory->heap->end_virtual_pointer = current->v_memory->heap->list->len + current->v_memory->heap->start_virtual_pointer; // actualizando end_pointer del heap
  out->addr = current->v_memory->heap->end_virtual_pointer;
  return 0;

  // while (1)
  // {
  //   if (current->v_memory->heap->list->len + size >= current->v_memory->heap->list->size)
  //   {
  //     current->v_memory->heap->list->size += PAGE_SIZE;
  //     Add_Free_Page_Frame_to_page_frames_indexed_by_v_pages(free_page_frames, current->pages_table, 1);
  //   }
  //   else
  //     break;
  // }
  // // ocupar una cantidad size en la lista
  // for (size_t i = 0; i < size; i++)
  // {
  //   push(current->v_memory->heap, 0);
  //   amount_new_elements_in_heap_before_add_new_v_page++;
  // }

  // out->addr = current->v_memory->heap->list->len - 1 - size + current->process.program->size + current->v_memory->stack->size_reserve;

  // if (amount_new_elements_in_heap_before_add_new_v_page > 128)
  // {
  //   size_t amount_of_new_pages_to_add = (amount_new_elements_in_heap_before_add_new_v_page % PAGE_SIZE) == 0 ? amount_new_elements_in_heap_before_add_new_v_page / PAGE_SIZE : (amount_new_elements_in_heap_before_add_new_v_page / PAGE_SIZE) + 1;
  //   amount_new_elements_in_heap_before_add_new_v_page = 0;
  //   Add_Free_Page_Frame_to_page_frames_indexed_by_v_pages(free_page_frames, current->pages_table, amount_of_new_pages_to_add);
  // }
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  set(current->v_memory->heap->list, ptr.addr, 0);

  // for (size_t i = get_vpn_of_va(ptr.addr) * PAGE_SIZE; i < (get_vpn_of_va(ptr.addr) + 1) * PAGE_SIZE; i++)
  // {
  //   if (get(current->v_memory->heap, i) == 1)
  //   {
  //     return 0;
  //   }
  // }

  // set(free_page_frames, get(current->pages_table, get_vpn_of_va(ptr.addr)), 0); // marcando como libre la pagina donde esta ubicado parte del proceso
  // set(current->pages_table, get_vpn_of_va(ptr.addr), -1);                       // eliminando pagina de la tabla de paginas del proceso
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  if (current->v_memory->stack->sp - 1 > current->process.program->size)
  {
    current->v_memory->stack->sp--;
    m_write(convert_va_in_pa(current->v_memory->stack->sp), val);
    return 0;
  }
  else
    return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (current->v_memory->stack->sp + 1 < current->v_memory->heap->start_virtual_pointer)
  {
    current->v_memory->stack->sp++;
    *out = m_read(convert_va_in_pa(current->v_memory->stack->sp));
    return 0;
  }
  else
    return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  *out = m_read(convert_va_in_pa(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  // no necesario marcar como ocupado en free_page_frame ya q se marco cuando reserve memoria para el en malloc
  if (!get(current->v_memory->heap->list, (size_t)addr))
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
  //     push(current->pages_table, i); // add nueva pagina a la tabla de pagina de current process
  //     break;
  //   }
  // }

  m_write(convert_va_in_pa(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) // checked // no estoy haciendo lo de set_owner etc
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
    process_pag_t *new_process = init_process_pag(process);
    p_push(list_of_process, new_process);
    current = new_process;
    update_pages_table(current, free_page_frames);
    m_set_owner(convert_va_in_pa(0), convert_va_in_pa(current->v_memory->heap->end_virtual_pointer) % PAGE_SIZE == 0 ? convert_va_in_pa(current->v_memory->heap->end_virtual_pointer) : convert_va_in_pa(current->v_memory->heap->end_virtual_pointer) + PAGE_SIZE - (convert_va_in_pa(current->v_memory->heap->end_virtual_pointer % PAGE_SIZE)));
  }
  else
  {
    // sustituir el current por el proceso actual si no es nuevo
    for (size_t i = 0; i < list_of_process->len; i++)
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
  // liberando paginas ocupadas ,por el proceso, en free_page_frames
  for (size_t i = 0; i < current->pages_table->len; i++)
  {
    set(free_page_frames, get(current->pages_table, i), 0);
  }

  for (size_t i = 0; i < current->pages_table->len;)
  {
    if (i + (size_t)PAGE_SIZE <= current->v_memory->heap->end_virtual_pointer)
    {
      m_unset_owner(convert_va_in_pa(i), convert_va_in_pa(i + (size_t)PAGE_SIZE));
    }
    else
      break;
    // else //no necesario ya q quiero liberar una pagina completa, aunq i+pagesize sea mayor q el end pointer, nama lo sera una cantidad menor o igual a 128 de bytes, luego
    // {
    //   m_unset_owner(convert_va_in_pa(i), convert_va_in_pa(current->v_memory->heap->end_virtual_pointer));
    //   break;
    // }
    i += (size_t)128;
  }

  for (size_t i = 0; i < list_of_process->len; i++)
  {
    if (p_get(list_of_process, i)->process.pid == current->process.pid)
    {
      p_deleteAt(list_of_process, i);
      break;
    }
  }
}
