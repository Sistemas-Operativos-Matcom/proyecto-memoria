#include "bnb_manager.h"

#include "stdio.h"

// #include "stack.h"
// #include "process_pag.h"
#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include "../tests.h"
#include "process_list.h"

process_pag_t *current_bnb;
process_List_t *list_of_process_bnb;
sizeList_t *free_bnb_page_frames;
int is_first_time_running_bnb = 1;

size_t convert_va_in_pa_bnb(addr_t va) // checked, analizar el -1 q esta ahi debajo,lo demas esta perfecto
{
  return (size_t)(get(current_bnb->pages_table, 0) * BNB_PAGE_SIZE) + ((size_t)va % (size_t)BNB_PAGE_SIZE); // posible error con el -1 pq la primera pagina es la 0,no la 1, entonces cuando es 7*BNB_PAGE_SIZE, quisas era 6*BNB_PAGE_SIZE
}

void Add_ONE_BNB_Free_Page_Frame_to_pages_table(sizeList_t *free_bnb_page_frames, sizeList_t *pages_table, int amount_of_new_pages_to_add) // checked,no big errors found
{
  int counter = 0;
  while (counter < amount_of_new_pages_to_add)
  {
    int first_free_page_frame_founded = 0;
    for (int i = 0; i < free_bnb_page_frames->len; i++)
    {
      if (get(free_bnb_page_frames, i) == 0)
      {
        first_free_page_frame_founded = i;
        break;
      }
    }

    push(pages_table, first_free_page_frame_founded);

    set(free_bnb_page_frames, first_free_page_frame_founded, 1);
    // printf("value: %zu  time: %d .", (addr_t)((size_t)(pages_table->len - 1) * (size_t)BNB_PAGE_SIZE), counter);
    m_set_owner(convert_va_in_pa_bnb((addr_t)((size_t)(pages_table->len - 1) * (size_t)BNB_PAGE_SIZE)), convert_va_in_pa_bnb((addr_t)((pages_table->len) * (size_t)BNB_PAGE_SIZE) - (size_t)1));
    counter++;
  }
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{

  if (is_first_time_running_bnb)
  {
    list_of_process_bnb = p_init();

    is_first_time_running_bnb = 0;
    free_bnb_page_frames = init();

    current_bnb = malloc(sizeof(process_pag_t));
  }
  else
  {
    p_reset(list_of_process_bnb); // no check
    reset(free_bnb_page_frames);
  }

  size_t size = (m_size() % (size_t)BNB_PAGE_SIZE) == 0 ? (m_size() / (size_t)BNB_PAGE_SIZE) : ((m_size() / (size_t)BNB_PAGE_SIZE) + (size_t)1); // creando lista de page frames libres, donde cada elemento es una pagina de 128 bytes de la memoria fisica

  for (size_t i = 0; i < size; i++)
  {
    push(free_bnb_page_frames, 0);
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{

  if (current_bnb->v_memory->heap->list->len != 0)
  {
    int counter_free_spaces_consecutive = 0;
    for (int i = 0; i < current_bnb->v_memory->heap->list->len; i++) // not check
    {
      if (counter_free_spaces_consecutive == (int)size)
      {
        for (int j = 0; j < (int)size; j++)
        {
          set(current_bnb->v_memory->heap->list, i - j, 1);
        }
        // dandole lugar en memoria fisica a esta pagina en caso de q en page_table no estuviera mapeada sustituyendo los -1 por nuevas paginas de memoria
        // update_pages_table(current_bnb, free_bnb_page_frames);
        // si no se hace lo de arriba entonces simplemente ocupo en 1 los lugares esos del heap ya que esta mapeada esta pagina del heap  en memoria fisica
        out->addr = (size_t)((i - (int)size - 1) + current_bnb->v_memory->heap->start_virtual_pointer);
        return 0;
      }
      if (get(current_bnb->v_memory->heap->list, i) == 0)
      {
        counter_free_spaces_consecutive++;
      }
      else
      {
        counter_free_spaces_consecutive = 0;
      }
    }
  }
  if (current_bnb->v_memory->heap->list->len + (int)size > BNB_PAGE_SIZE)
  {
    printf("memoria agotada");
    exit(1);
  }
  // add memoria to the final of the heap
  for (int i = 0; i < (int)size; i++) // check
  {
    push(current_bnb->v_memory->heap->list, 1);
  }

  current_bnb->v_memory->heap->end_virtual_pointer = current_bnb->v_memory->heap->list->len + current_bnb->v_memory->heap->start_virtual_pointer; // actualizando end_pointer del heap
  out->addr = (size_t)current_bnb->v_memory->heap->end_virtual_pointer - size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  set(current_bnb->v_memory->heap->list, ptr.addr, 0);
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (current_bnb->v_memory->stack->sp - 1 >= (int)current_bnb->process.program->size)
  {

    // printf("sp pointer: %zu", convert_va_in_pa_bnb((size_t)current_bnb->v_memory->stack->sp));
    m_write(convert_va_in_pa_bnb((size_t)(current_bnb->v_memory->stack->sp)), val); // arreglar convert va in pa
    out->addr = (size_t)current_bnb->v_memory->stack->sp;
    current_bnb->v_memory->stack->sp--;

    return 0;
  }
  else
    return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (current_bnb->v_memory->stack->sp + 1 <= current_bnb->v_memory->heap->start_virtual_pointer)
  {
    current_bnb->v_memory->stack->sp++;
    *out = m_read(convert_va_in_pa_bnb((size_t)current_bnb->v_memory->stack->sp));

    return 0;
  }
  else
    return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  *out = m_read(convert_va_in_pa_bnb(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (0 == get(current_bnb->v_memory->heap->list, (int)((size_t)addr - (size_t)current_bnb->v_memory->heap->start_virtual_pointer))) // si no esta ocupado
  {
    printf("esa posicion no esta reservada");
    return 1;
  }

  m_write(convert_va_in_pa_bnb(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{

  int is_new_process = 1;
  for (int i = 0; i < list_of_process_bnb->len; i++)
  {
    if (p_get(list_of_process_bnb, i)->process.pid == process.pid)
    {

      is_new_process = 0;
    }
  }

  // agregar el proceso a la lista de procesos si es un proceso nuevo
  if (is_new_process)
  {

    process_pag_t *new_process = init_process_pag(process);
    p_push(list_of_process_bnb, new_process);

    current_bnb = new_process;

    // printf("veams si init process pag pincha PID:%d ,program_size: %d,stack size reserved: %d, heap end pointer: %d , stack pointer: %d  ", current_bnb->process.pid, current_bnb->process.program->size, current_bnb->v_memory->stack->size_reserve, current_bnb->v_memory->heap->end_virtual_pointer, current_bnb->v_memory->stack->sp);
    Add_ONE_BNB_Free_Page_Frame_to_pages_table(free_bnb_page_frames, current_bnb->pages_table, 1);

    // creo qq esta linea ta mal pq ya lo toy haciendo en update pages ,m_set_owner(convert_va_in_pa_bnb((addr_t)0), convert_va_in_pa_bnb(current_bnb->v_memory->heap->end_virtual_pointer) % BNB_PAGE_SIZE == 0 ? convert_va_in_pa_bnb(current_bnb->v_memory->heap->end_virtual_pointer) : convert_va_in_pa_bnb(current_bnb->v_memory->heap->end_virtual_pointer) + BNB_PAGE_SIZE - (convert_va_in_pa_bnb(current_bnb->v_memory->heap->end_virtual_pointer % BNB_PAGE_SIZE)));
  }
  else
  {
    // sustituir el current_bnb por el proceso actual si no es nuevo
    for (int i = 0; i < list_of_process_bnb->len; i++)
    {
      if (p_get(list_of_process_bnb, i)->process.pid == process.pid)
      {

        current_bnb = p_get(list_of_process_bnb, i);

        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  // liberando paginas ocupadas ,por el proceso, en free_bnb_page_frames

  set(free_bnb_page_frames, get(current_bnb->pages_table, 0), 0);

  m_unset_owner(convert_va_in_pa_bnb((size_t)(get(current_bnb->pages_table, 0) * BNB_PAGE_SIZE)), convert_va_in_pa_bnb((size_t)((get(current_bnb->pages_table, 0) + 1) * BNB_PAGE_SIZE + 1)));

  for (int i = 0; i < list_of_process_bnb->len; i++)
  {
    if (p_get(list_of_process_bnb, i)->process.pid == current_bnb->process.pid)
    {
      p_deleteAt(list_of_process_bnb, i);
      break;
    }
  }
}
