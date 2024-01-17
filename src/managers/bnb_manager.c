#include "bnb_manager.h"
#include "stdio.h"

#define bound 1024

typedef struct Process_context
{
  int pid;
  size_t heap_p;
  addr_t base;
  free_list *heap;
  addr_t SP;
  byte used;
} Process_context;

/*
 Para manejar los procesos uso "Procesos"
*/
static size_t process_index;
static Process_context *Procesos;
static size_t process_count;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  process_count = m_size() / bound;
  Procesos = (Process_context *)malloc(sizeof(Process_context) * process_count);
  
  for (size_t i = 0; i < process_count; i++)
  {
    Procesos[i].used = 0;
    Procesos[i].heap_p = NULL;
    Procesos[i].base = i * bound;
    Procesos->SP = (i + 1) * bound;
  }
}
// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.

/*
memory malloc y memory free se los delego al manejador de espacios del heap
*/
int m_bnb_malloc(size_t size, ptr_t *out)
{
  int result = memory_malloc(Procesos[process_index].heap, size, out);
  out->addr += Procesos[process_index].heap_p;

  return !result;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  ptr.addr -= Procesos[process_index].heap_p;
  int result = memory_free(Procesos[process_index].heap, ptr);

  return !result;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  //el cuando crece el stack la ultima seccion de espacio libre del heap pierde 1 byte
  int result = memory_reduce(Procesos[process_index].heap);

  if (result)
  {
    Procesos[process_index].SP--;
    m_write(out, val);
    out->addr = Procesos[process_index].SP;

    return !result;
  }

  return result;
}

// Quita un elemento del stack
//similar al push
int m_bnb_pop(byte *out)
{
  if (Procesos[process_index].SP >= Procesos[process_index].base + bound)
    return 0;

  *out = m_read(Procesos[process_index].SP);
  //al final del heap hay mas espacio
  memory_expand(Procesos[process_index].heap, 1);
  Procesos[process_index].SP++;

  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  *out = m_read(Procesos[process_index].base + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  m_write(Procesos[process_index].base + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  // cambiar el current process y si el process pid no existe en la lista de procesos, añadirlo
  int founded = 0;
  set_curr_owner(process.pid);

  for (size_t i = 0; i < process_count; i++)
  {
    if (Procesos[i].used == 1 && Procesos[i].pid == process.pid)
    {
      founded = 1;
      process_index = i;
    }
  }

  if (founded == 0)
  {
    for (size_t i = 0; i < process_count; i++)
    {
      if (Procesos[i].used == 0)
      {
        Procesos[i].heap = new_free_list(bound - process.program->size);
        Procesos[i].heap_p = malloc(sizeof(process_t));
        Procesos[i].pid = process.pid;
        Procesos[i].heap_p = process.program->size;
        Procesos[i].used = 1;
        process_index = i;

        m_set_owner(Procesos[i].base, Procesos[i].SP);
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (size_t i = 0; i < process_count; i++)
  {
    if (Procesos[i].pid == process.pid)
    {
      Procesos[i].used = 0;
      free(Procesos[i].heap);
      Procesos[i].SP = Procesos[process_index].base + bound;

      m_unset_owner(Procesos[i].base, Procesos[process_index].base + bound);
      break;
    }
  }
}
