#include "pag_manager.h"
#include "memory.h"
#include "stdio.h"
#include "list.h"

array_list *memory_free_start;
array_list *memory_free_size;
array_list *running_process;
process_t current_process;
long page_size = 128;
long stack_size = 32;

// SIEMPRE VOY A PONER PRIMERO EL CODIGO, LUEGO EL STACK, Y LUEGO HEAP

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  int pages = m_size() / page_size;
  running_process = create_list();
  memory_free_start = create_list();
  memory_free_size = create_list();
  append(memory_free_start, 0);
  append(memory_free_size, pages);
}

// VERIFICA SI PROC ESTA CORRIENDO
int is_running(process_t proc)
{
  for (int i = 0; i < running_process->size; i++)
  {
    // printf("%d vs %d\n", running_procs->array[i], process.pid);
    if (running_process->array[i] == proc.pid)
      return 1;
  }
  return 0;
}

// ESTE METODO REVISA SI HAY ESPACIOS LIBRES CONSECUTIVOS Y
// LOS MEZCLA, ASI COMO ELIMINA ESPACIOS = 0
void update_free(array_list *listSTART, array_list *listSIZE)
{
  for (int i = 0; i < listSTART->size - 1; i++)
  {
    if (listSTART->array[i] + listSIZE->array[i] == listSTART->array[i + 1])
    {
      listSIZE->array[i] += listSIZE->array[i + 1];
      delete (listSTART, i + 1);
      delete (listSIZE, i + 1);
      i--;
    }
  }
  for (int i = 0; i < listSTART->size; i++)
  {
    if (listSIZE->array[i] == 0)
    {
      delete (listSTART, i);
      delete (listSIZE, i);
      i--;
    }
  }
}

// ACA AGREGO ESPACIO LIBRE
void add_freespace(array_list *listSTART, array_list *listSIZE, int addr, int size)
{
  for (int i = 0; i < listSTART->size; i++)
  {
    if (addr < listSTART->array[i])
    {
      insert(listSTART, addr, i);
      insert(listSIZE, size, i);
      update_free(listSTART, listSIZE);
      return;
    }
  }
  append(listSTART, addr);
  append(listSIZE, size);
  update_free(listSTART, listSIZE);
}

// ESTO PIDE OCUPAR TANTO ESPACIO, DEVUELVE 1 SI LO LOGRA Y 0 SI NO
int ocupate_space(array_list *listSTART, array_list *listSIZE, int size, ptr_t *out)
{
  for (int i = 0; i < listSIZE->size; i++)
  {
    if (size <= listSIZE->array[i])
    {
      out->addr = (size_t)listSTART->array[i];
      out->size = (size_t)size;
      listSIZE->array[i] -= size;
      listSTART->array[i] += size;
      update_free(listSTART, listSIZE);
      return 1;
    }
  }
  return 0;
}

// AGREGA UNA PAGINA
void add_page()
{
  // printf("added page");
  ptr_t outt;
  ocupate_space(memory_free_start, memory_free_size, 1, &outt);
  add_freespace(current_process.start_free, current_process.size_free, current_process.blocksused->size * page_size, page_size);
  append(current_process.blocksused, (int)outt.addr);
  update_free(current_process.start_free, current_process.size_free);
  m_set_owner((size_t)outt.addr * (size_t)page_size, (size_t)outt.addr * (size_t)page_size + (size_t)(page_size - 1));
}

// ESTO BUSCA ESPACIO LIBRE Y AGREGA PAGINAS MIENTRAS NO HAYA
int allocate_space_metralla(int size, ptr_t *out)
{
  int logrado = 0;
  while (!logrado)
  {
    logrado = ocupate_space(current_process.start_free, current_process.size_free, size, out);
    if (!logrado)
      add_page();
  }
  return 0;
}

int to_memory_address(int addr)
{
  return current_process.blocksused->array[addr / page_size] * page_size + addr % page_size;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  // printf("pidio alocar");
  return allocate_space_metralla((int)size, out);
}

void free_unused_pages()
{
  set_curr_owner(NO_ONWER);
  for (int i = 0; i < current_process.blocksused->size; i++)
  {
    for (int j = 0; j < current_process.start_free->size; j++)
    {
      if (current_process.start_free->array[j] <= i * page_size &&
          current_process.start_free->array[j] +
                  current_process.size_free->array[j] >=
              (i + 1) * page_size)
      {
        m_set_owner((size_t)(i * page_size), (size_t)(((i + 1) * page_size) - 1));
        insert(current_process.start_free, (i + 1) * page_size, j + 1);
        insert(current_process.size_free, current_process.start_free->array[j] + current_process.size_free->array[j] - (i + 1) * page_size, j + 1);
        current_process.size_free->array[j] = i * page_size - current_process.start_free->array[j];
        add_freespace(memory_free_start, memory_free_size, current_process.blocksused->array[i], 1);
        update_free(memory_free_start, memory_free_size);
        current_process.blocksused->array[i] = -1;
        break;
      }
    }
  }
  set_curr_owner(current_process.pid);
  update_free(current_process.start_free, current_process.size_free);
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  add_freespace(current_process.start_free, current_process.size_free, (int)ptr.addr, (int)ptr.size);
  free_unused_pages();
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  if (*current_process.sp >= 0 && *current_process.sp < stack_size)
  {
    m_write((size_t)to_memory_address(*current_process.stack_base + *current_process.sp), val);
    out->addr = (size_t)*current_process.stack_base + *current_process.sp;
    *current_process.sp += 1;
    return 0;
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  *current_process.sp -= 1;
  if (*current_process.sp >= 0 && *current_process.sp < stack_size)
  {
    *out = m_read((size_t)to_memory_address(*current_process.stack_base + *current_process.sp));
    return 0;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  if (addr >= current_process.program->size + stack_size)
  {
    for (int i = 0; i < current_process.start_free->size; i++)
    {
      if (current_process.start_free->array[i] <= (int)addr && current_process.start_free->array[i] + current_process.size_free->array[i] > (int)addr)
      {
        return 1;
      }
    }

    *out = m_read((size_t)to_memory_address((int)addr));
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (addr >= current_process.program->size + stack_size)
  {
    for (int i = 0; i < current_process.start_free->size; i++)
    {
      if (current_process.start_free->array[i] <= (int)addr && current_process.start_free->array[i] + current_process.size_free->array[i] > (int)addr)
      {
        return 1;
      }
    }

    m_write((size_t)to_memory_address((int)addr), val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  current_process = process;
  ptr_t out;
  if (!is_running(process))
  {
    allocate_space_metralla((int)current_process.program->size, &out);
    allocate_space_metralla(stack_size, &out);
    *current_process.stack_base = current_process.program->size;
    *current_process.sp = 0;
    append(running_process, process.pid);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  set_curr_owner(NO_ONWER);
  for (int i = 0; i < process.blocksused->size; i++)
  {
    if (process.blocksused->array[i] != -1)
    {
      add_freespace(memory_free_start, memory_free_size, page_size * process.blocksused->array[i], page_size);
      m_set_owner((size_t)(page_size * process.blocksused->array[i]), (size_t)(page_size * process.blocksused->array[i] + (page_size - 1)));
    }
  }
  for (int i = 0; i < running_process->size; i++)
  {
    if (current_process.pid == running_process->array[i])
    {
      delete (running_process, i);
      break;
    }
  }

  set_curr_owner(current_process.pid);
}
