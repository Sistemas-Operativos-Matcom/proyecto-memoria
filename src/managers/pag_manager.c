#include "pag_manager.h"

#include "stdio.h"

const size_t page_size = 32;
/*
pages informa de las paginas que estan usadas
process_list tiene todos los procesos
p index es el indice del procesos actual en la lista de procesos
*/
static int *pages;
static int pages_count;
static List_process *process_list;
static int P_index;

// recibe un heap o un stack y e intenta darle una pagina
int add_free_page(List_int *book)
{
  int free_page_index = -1;

  for (int i = 0; i < pages_count; i++)
  {
    if (!pages[i])
    {
      free_page_index = i;
      break;
    }
  }
  if (free_page_index == -1)
  {
    return 0;
  }
  else
  {
    pages[free_page_index] = 1;
    Push(free_page_index, book);
    m_set_owner(free_page_index * page_size, (free_page_index + 1) * page_size - 1);
    
    return 1;
  }
}
// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  process_list = new_list_process();
  pages_count = m_size() / page_size;
  pages = malloc(pages_count * sizeof(int));
  
  for (int i = 0; i < pages_count; i++)
  {
    pages[i] = 0;
  }
  P_index = 0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.

//le da paginas al heap hasta que pueda reservar espacio
int m_pag_malloc(size_t size, ptr_t *out)
{
  while (!memory_malloc(Get_index_process(P_index, process_list).heap, size, out))
  {
    if (!add_free_page(Get_index_process(P_index, process_list).heap_pages))
      return 1;
    
    memory_expand(Get_index_process(P_index, process_list).heap, page_size);
  }
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  return !memory_free(Get_index_process(P_index, process_list).heap, ptr);
}

// Agrega un elemento al stack

//el stack "esta" al final de la ram, si las paginas estan llenas se le da una mas
int m_pag_push(byte val, ptr_t *out)
{
  int full = 0;
  
  if (process_list->list_start[P_index].SP >= process_list->list_start[P_index].stack_pages->length * page_size)
    full = !add_free_page(process_list->list_start[P_index].stack_pages);
  
  if (full)
    return 1;
  
  m_pag_store(m_size() - process_list->list_start[P_index].SP, val);
  process_list->list_start[P_index].SP = process_list->list_start[P_index].SP + 1;
  
  return 0;
}

// Quita un elemento del stack
/*
  el stack pointer apunta al final del ultimo elemento en las paginas del stack
  crece y decrece al revez que el SP normal
*/
int m_pag_pop(byte *out)
{
  if (process_list->list_start[P_index].SP == 0)
    return 1;

  process_list->list_start[P_index].SP --;
  //si el stack usa una pagina mas de laS que le hace falta entonces la pierde
  if (process_list->list_start[P_index].SP < (process_list->list_start[P_index].stack_pages->length - 1) * page_size)
  {
    int stack_size = process_list->list_start[P_index].stack_pages->length;

    int last_page = process_list->list_start[P_index].stack_pages->list_start[stack_size - 1];

    m_unset_owner(last_page * page_size, (last_page + 1) * page_size - 1);

    pages[last_page] = 0;

    Delete(stack_size - 1, process_list->list_start[P_index].stack_pages);
  }

  m_pag_load(m_size() - process_list->list_start[P_index].SP, out);

  return 0;
}

// Carga el valor en una dirección determinada
/*la lectura la hace del heap o del stack en dependencia de si el address cae 
  en sus espacios y despues se hace la conversion del address para sacar el valor.
*/
int m_pag_load(addr_t addr, byte *out)
{
  if (addr < process_list->list_start[P_index].heap_pages->length * page_size)
  {
    int real_page = process_list->list_start[P_index].heap_pages->list_start[(int)(addr / page_size)];
    *out = m_read(real_page * page_size + addr % page_size);
    return 0;
  }
  //Para el programa su stack esta al final de la memoria
  else if (m_size() - addr < process_list->list_start[P_index].stack_pages->length * page_size)
  {
    addr = m_size() - addr;
    int real_page = process_list->list_start[P_index].stack_pages->list_start[(int)(addr / page_size)];
    *out = m_read(real_page * page_size + addr % page_size);
    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
//similar al load
int m_pag_store(addr_t addr, byte val)
{
  if (addr < process_list->list_start[P_index].heap_pages->length * page_size)
  {
    int real_page = process_list->list_start[P_index].heap_pages->list_start[(int)(addr / page_size)];
    m_write(real_page * page_size + addr % page_size, val);
    return 0;
  }
  else if (m_size() - addr < process_list->list_start[P_index].stack_pages->length * page_size)
  {
    addr = m_size() - addr;
    int real_page = process_list->list_start[P_index].stack_pages->list_start[(int)(addr / page_size)];
    m_write(real_page * page_size + addr % page_size, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
//funciona parecido que el switch dde bnb pero necesita inicializar el heap y stack
void m_pag_on_ctx_switch(process_t process)
{
  set_curr_owner(process.pid);
  for (int i = 0; i < process_list->length; i++)
  {
    if (Get_index_process(i, process_list).process_pid == process.pid)
    {
      P_index = i;
      return;
    }
  }
  Process value;
  value.process_pid = process.pid;

  value.heap_pages = new_list_int();
  add_free_page(value.heap_pages);
  value.heap = new_free_list(page_size);

  value.stack_pages = new_list_int();
  add_free_page(value.heap_pages);
  value.SP = 0;

  Push_process(value, process_list);
  P_index = process_list->length - 1;

  //el puntero es solo para ayudar a reservar el espacio del programa en el heap
  ptr_t *temp = malloc(sizeof(ptr_t));
  m_pag_malloc(process.program->size, temp);
  free(temp);
}

// Notifica que un proceso ya terminó su ejecución
// le hace unset a todas las paginas del heap y stack y las pone como libres
void m_pag_on_end_process(process_t process)
{
  int process_index = -1;
  for (int i = 0; i < process_list->length; i++)
  {
    if (process_list->list_start[i].process_pid == process.pid)
    {
      process_index = i;
      break;
    }
  }
  
  //hay que calcular la verdadera direccion de las paginas
  int pageframe = 0;
  
  for (int i = 0; i < process_list->list_start[process_index].heap_pages->length; i++)
  {
    pages[process_list->list_start[process_index].heap_pages->list_start[i]] = 0;

    pageframe = Pop(process_list->list_start[process_index].heap_pages);

    m_unset_owner(pageframe * page_size, (pageframe + 1) * page_size - 1);
  }

  for (int i = 0; i < process_list->list_start[process_index].stack_pages->length; i++)
  {
    pages[process_list->list_start[process_index].stack_pages->list_start[i]] = 0;

    pageframe = Pop(process_list->list_start[process_index].stack_pages);

    m_unset_owner(pageframe * page_size, (pageframe + 1) * page_size - 1);
  }

  free(process_list->list_start[process_index].heap);
  free(process_list->list_start[process_index].heap_pages);
  free(process_list->list_start[process_index].stack_pages);

  Delete_process(process_index, process_list);
}
