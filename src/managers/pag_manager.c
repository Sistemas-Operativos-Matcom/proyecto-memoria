#include "pag_manager.h"

#include "stdio.h"

static proc_info_t *proc;
static int *virtual_mem;

static int *pfn;
static int curr_procc_pid;
static int actual_process_index;
static size_t num_page_frames;

#define PAGES 4
#define block_sz 256
#define stack_sz (PAGES * block_sz)
#define code_sz 1
#define KB(size) size / block_sz

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
 
  if (virtual_mem != NULL)
  {
    free(virtual_mem);
    virtual_mem = NULL;
  }
  if (proc != NULL)
  {
    free(proc);
    proc = NULL;
  }
  if (pfn != NULL)
  {
    free(pfn);
    pfn = NULL;
  }

  num_page_frames = KB(m_size());

  proc = (proc_info_t *)malloc(num_page_frames * sizeof(proc_info_t));
  pfn = (int *)malloc(num_page_frames * sizeof(num_page_frames));
  virtual_mem = (int *)malloc(num_page_frames * sizeof(int));

  for (size_t i = 0; i < num_page_frames; i++)
  {
    proc[i].assigned = 0;
    proc[i].page_table = (size_t *)malloc(PAGES * sizeof(size_t));

    for (size_t j = 0; j < PAGES; j++)
    {
      proc[i].page_table[j] = -1;
    }

    proc[i].user = -1;

    proc[i].heap = 0;
    proc[i].stack = stack_sz;

    virtual_mem[i] = -1;
    pfn[i] = i;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  int flag = 1;

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (virtual_mem[i] == -1)
    {
      flag = 0;

      out->addr = (i * block_sz);
      out->size = 1;

      virtual_mem[i] = curr_procc_pid;
      proc[actual_process_index].page_table[0] = i;
      m_set_owner(i * block_sz, (i + 1) * block_sz - 1);

      break;
    }
  }

  return flag;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  size_t actual_page_frame = (size_t)(ptr.addr / block_sz);
  size_t end_page_frame = (size_t)((ptr.addr + ptr.size) / block_sz);
  int flag = 0;

  if (end_page_frame >= num_page_frames || ptr.size > proc[actual_process_index].heap)
  {
    return 1;
  }

  for (size_t i = actual_page_frame; i < end_page_frame; i++)
  {
    if (virtual_mem[i] != curr_procc_pid)
    {
      flag = 1;
      break;
    }
  }

  if (flag)
  {
    return 1;
  }

  proc[actual_process_index].heap -= ptr.size;
  size_t page_frame;

  for (size_t i = 0; i < PAGES; i++)
  {
    page_frame = proc[actual_process_index].page_table[i];

    if (page_frame > actual_page_frame && page_frame <= end_page_frame)
    {
      m_unset_owner(page_frame * block_sz, (page_frame + 1) * block_sz - 1);
      proc[actual_page_frame].page_table[i] = -1;
    }
  }

  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  size_t aux = stack_sz - proc[actual_process_index].stack;
  size_t page;

  if (proc[actual_process_index].heap + 1 == proc[actual_process_index].stack)
  {
    return 1;
  }

  // Agrega una nueva pagina a la memoria del programa actual
  if (aux % block_sz == 0)
  {
    for (size_t i = 0; i < num_page_frames; i++)
    {
      if (virtual_mem[i] == -1)
      {
        virtual_mem[i] = curr_procc_pid;

        page = PAGES - (size_t)(aux / block_sz) - 1;
        proc[actual_process_index].page_table[page] = i;
        m_set_owner(i * block_sz, (i + 1) * block_sz - 1);

        break;
      }
    }
  }

  proc[actual_process_index].stack -= 1;
  aux += 1;
  page = PAGES - (size_t)(aux / block_sz) - 1;
  size_t page_frame = proc[actual_process_index].page_table[page];
  size_t addr = (page_frame * block_sz) + (aux % block_sz);

  m_write(addr, val);

  out->addr = proc[actual_process_index].stack;

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (proc[actual_process_index].stack == stack_sz)
  {
    return 1;
  }

  // Encuentra la direccion fisica
  size_t aux = stack_sz - proc[actual_process_index].stack;
  size_t page = PAGES - (size_t)(aux / block_sz) - 1;
  size_t page_frame = proc[actual_process_index].page_table[page];
  size_t addr = (page_frame * block_sz) + (aux % block_sz);

  *out = m_read(addr);

  proc[actual_process_index].stack += 1;

  // En caso de terminar con el page_frame actual lo libera
  if (aux % block_sz == 0)
  {
    virtual_mem[page_frame] = -1;
    proc[actual_process_index].page_table[page] = -1;
    m_unset_owner(page_frame * block_sz, (page_frame + 1) * block_sz - 1);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  size_t actual_page = (size_t)(addr / block_sz);

  if (virtual_mem[actual_process_index] == curr_procc_pid)
  {
    *out = m_read(addr);

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  size_t actual_page = (size_t)(addr / block_sz);

  if (virtual_mem[actual_page] == curr_procc_pid)
  {
    m_write(addr, val);

    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  curr_procc_pid = process.pid;

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (virtual_mem[i] == process.pid)
    {
      actual_process_index = i;
      return;
    }
  }

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (!proc[i].assigned)
    {
      proc[i].assigned = 1;
      proc[i].user = process.pid;
      virtual_mem[i] = process.pid;
      actual_process_index = i;

      break;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (proc[i].user == process.pid)
    {
      proc[i].assigned = 0;
      int page_frame;

      for (size_t j = 0; j < PAGES; j++)
      {
        page_frame = proc[i].page_table[j];

        if (page_frame != -1)
        {
          virtual_mem[page_frame] = 0;
        }

        proc[i].page_table[j] = -1;
        m_unset_owner(i * block_sz, (i + 1) * block_sz - 1);
      }

      proc[i].user = -1;
      proc[i].heap = 0;
      proc[i].stack = stack_sz;
    }
  }
}