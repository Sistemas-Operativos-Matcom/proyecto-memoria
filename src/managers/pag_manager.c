#include "pag_manager.h"

#include "stdio.h"

static int *pages;
static int *index;
static int current_proc_id;
static int current_id;
static size_t num_mp;

#define pag_size 256

typedef struct current_process
{
  int id;
  size_t *pag_table;
  size_t heap;
  size_t stack;
  int exec;
} current_process_t;

static current_process_t *procs;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  free(procs);
  free(pages);
  free(index);

  num_mp = m_size() / pag_size;
  procs = (current_process_t*)malloc(sizeof(current_process_t) * num_mp);
  pages = (int*)malloc(sizeof(int) * num_mp);
  index = (int*)malloc(sizeof(int) * num_mp);

  for(size_t i = 0; i < num_mp; i++)
  {
    current_process_t *current_proc = &procs[i];
    current_proc->exec = 0;
    current_proc->pag_table = (size_t*)malloc(sizeof(size_t) * 4);

    for(size_t j = 0; j < 4; j++)
    {
      current_proc->pag_table[j] = -1;
    }

    current_proc->heap = 0;
    current_proc->stack = pag_size * 4;
    current_proc->id = -1;

    pages[i] = -1;
    index[i] = i;
  }
}


// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int tp = 1;

  for(size_t i = 0; i < num_mp; i++)
  {
    if(pages[i] == 1)
    {
      tp = 0;
      pages[i] = current_proc_id;
      out->addr = pag_size * i;
      out->size = 1;
      procs[current_id].pag_table[0] = i;
      m_set_owner(pag_size * i, (i + 1) * pag_size - 1);
      
      break;
    }
  }
  return tp;
}


// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  int tp = 0;
  size_t current_pf = (size_t)(ptr.addr / pag_size);
  size_t end_pf = (size_t)((ptr.addr + ptr.size) / pag_size);
  
  if(end_pf >= num_mp || ptr.size > procs[current_id].heap)
  {
    return 1;
  }

  for(size_t i = current_pf; i < end_pf; i++)
  {
    if(pages[i] != current_proc_id)
    {
      tp = 1;
      return tp;
    }
  }
  procs[current_id].heap -= ptr.size;

  for(size_t i = 0; i < 4; i++)
  {
    size_t pf = procs[current_id].pag_table[i];

    if(pf > current_pf && pf <= end_pf)
    {
      m_unset_owner(pf * pag_size, (pf + 1) * pag_size - 1);
      procs[current_pf].pag_table[i] = -1;
    }
  }
  return 0;
}


// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  size_t page;
  size_t stack_size = (pag_size * 4) - procs[current_id].stack;

  if(procs[current_id].heap + 1 == procs[current_id].stack)
  {
    return 1;
  }

  if(stack_size % pag_size == 0)
  {
    for(size_t i = 0; i < num_mp; i++)
    {
      if(pages[i] == -1)
      {
        page = 4 - (size_t)(stack_size / pag_size) - 1;
        pages[i] = current_proc_id;
        procs[current_id].pag_table[page] = i;
        m_set_owner(pag_size * i, (i + 1) * pag_size - 1);

        break;
      }
    }
  }

  stack_size += 1;
  page = 4 - (size_t)(stack_size / pag_size) - 1;

  procs[current_id].stack = -1;

  size_t pf = procs[current_id].pag_table[page];
  size_t address = (pag_size * pf) + (stack_size % pag_size);

  m_write(address, val);
  out->addr = procs[current_id].stack;

  return 0;
}


// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if(procs[current_id].stack == (pag_size * 4))
  {
    return 1;
  }

  size_t stack_size = (pag_size * 4) - procs[current_id].stack;
  size_t page = 4 - (size_t)(stack_size / pag_size) - 1;
  size_t pf = procs[current_id].pag_table[page];
  size_t address = (stack_size % pag_size) + (pag_size * pf);

  *out = m_read(address);

  procs[current_id].stack += 1;

  if(stack_size % pag_size == 0)
  {
    procs[current_id].pag_table[page] = -1;
    pages[pf] = -1;

    m_unset_owner(pag_size * pf, (pf + 1) * pag_size - 1);
  }

  return 0; 
}


// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  size_t current_page = (size_t)(addr / pag_size);

  if(pages[current_page] == current_proc_id)
  {
    *out = m_read(addr);
    return 0;
  }

  return 1;
}


// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  size_t current_page = (size_t)(addr / pag_size);

  if(pages[current_page] == current_proc_id)
  {
    m_write(addr, val);
    return 0;
  }

  return 1;
}


// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  current_proc_id = process.pid;

  for(size_t i = 0; i < num_mp; i++)
  {
    if(pages[i] == current_proc_id)
    {
      current_id = i;

      return;
    }
  }

  for(size_t i = 0; i < num_mp; i++)
  {
    if(!procs[i].exec)
    {
      procs[i].id = process.pid;
      procs[i].exec = 1;
      pages[i] = process.pid;
      current_id = i;

      break;
    }
  }
}


// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for(size_t i = 0; i < num_mp; i++)
  {
    if(procs[i].id == process.pid)
    {
      procs[i].exec = 0;
      int pf;

      for(size_t j = 0; j < 4; j++)
      {
        pf = procs[i].pag_table[j];

        if(pf != -1)
        {
          pages[pf] = 0;
        }

        procs[i].pag_table[j] = -1;
        m_unset_owner(pag_size * i, (i + 1) * pag_size - 1);
      }

      procs[i].heap = 0;
      procs[i].stack = (4 * pag_size);
      procs[i].id = -1;
    }
  }
}
