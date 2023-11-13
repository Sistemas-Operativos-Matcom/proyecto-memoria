#include "pag_manager.h"

#include "stdio.h"

static info_proceso_t *procesos;
static int *memoria_virtual;

static int *pfn;// Contiene los PFN de cada page_frame
static int actual_process_pid;
static int actual_process_index;
static size_t num_page_frames;

#define PAGES 4
#define tam_Bloque 256
#define tam_Code 1
#define tam_Stack (PAGES * tam_Bloque)
#define KB(size) size / tam_Bloque

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
 
  if (memoria_virtual != NULL)
  {
    free(memoria_virtual);
    memoria_virtual = NULL;
  }
  if (procesos != NULL)
  {
    free(procesos);
    procesos = NULL;
  }
  if (pfn != NULL)
  {
    free(pfn);
    pfn = NULL;
  }

  num_page_frames = KB(m_size());

  procesos = (info_proceso_t *)malloc(num_page_frames * sizeof(info_proceso_t));
  pfn = (int *)malloc(num_page_frames * sizeof(num_page_frames));
  memoria_virtual = (int *)malloc(num_page_frames * sizeof(int));

  for (size_t i = 0; i < num_page_frames; i++)
  {
    // inicializa las variables para cada proceso
    procesos[i].asignado = 0;
    procesos[i].page_table = (size_t *)malloc(PAGES * sizeof(size_t));

    for (size_t j = 0; j < PAGES; j++)
    {
      procesos[i].page_table[j] = -1;
    }

    procesos[i].usuario = -1;

    procesos[i].heap = 0;
    procesos[i].stack = tam_Stack;

    // Marca el page_frame como no usado
    memoria_virtual[i] = -1;
    pfn[i] = i;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  int encontrado = 1;

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (memoria_virtual[i] == -1)
    {
      encontrado = 0;

      out->addr = (i * tam_Bloque);
      out->size = 1;

      memoria_virtual[i] = actual_process_pid;
      procesos[actual_process_index].page_table[0] = i;
      m_set_owner(i * tam_Bloque, (i + 1) * tam_Bloque - 1);

      break;
    }
  }

  return encontrado;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  size_t actual_page_frame = (size_t)(ptr.addr / tam_Bloque);
  size_t end_page_frame = (size_t)((ptr.addr + ptr.size) / tam_Bloque);
  int encontrado = 0;

  if (end_page_frame >= num_page_frames || ptr.size > procesos[actual_process_index].heap)
  {
    return 1;
  }

  for (size_t i = actual_page_frame; i < end_page_frame; i++)
  {
    if (memoria_virtual[i] != actual_process_pid)
    {
      encontrado = 1;
      break;
    }
  }

  if (encontrado)
  {
    return 1;
  }

  procesos[actual_process_index].heap -= ptr.size;
  size_t page_frame;

  for (size_t i = 0; i < PAGES; i++)
  {
    page_frame = procesos[actual_process_index].page_table[i];

    if (page_frame > actual_page_frame && page_frame <= end_page_frame)
    {
      m_unset_owner(page_frame * tam_Bloque, (page_frame + 1) * tam_Bloque - 1);
      procesos[actual_page_frame].page_table[i] = -1;
    }
  }

  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  size_t tam_stack = tam_Stack - procesos[actual_process_index].stack;
  size_t page;

  if (procesos[actual_process_index].heap + 1 == procesos[actual_process_index].stack)
  {
    return 1;
  }

  // Agrega una nueva pagina a la memoria del programa actual
  if (tam_stack % tam_Bloque == 0)
  {
    for (size_t i = 0; i < num_page_frames; i++)
    {
      if (memoria_virtual[i] == -1)
      {
        memoria_virtual[i] = actual_process_pid;

        page = PAGES - (size_t)(tam_stack / tam_Bloque) - 1;
        procesos[actual_process_index].page_table[page] = i;
        m_set_owner(i * tam_Bloque, (i + 1) * tam_Bloque - 1);

        break;
      }
    }
  }

  // Agrega un nuevo elemento al stack
  procesos[actual_process_index].stack -= 1;
  tam_stack += 1;
  page = PAGES - (size_t)(tam_stack / tam_Bloque) - 1;
  size_t page_frame = procesos[actual_process_index].page_table[page];
  size_t addr = (page_frame * tam_Bloque) + (tam_stack % tam_Bloque);

  m_write(addr, val);

  out->addr = procesos[actual_process_index].stack;

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (procesos[actual_process_index].stack == tam_Stack)
  {
    return 1;
  }

  // Encuentra la direccion fisica
  size_t tam_stack = tam_Stack - procesos[actual_process_index].stack;
  size_t page = PAGES - (size_t)(tam_stack / tam_Bloque) - 1;
  size_t page_frame = procesos[actual_process_index].page_table[page];
  size_t addr = (page_frame * tam_Bloque) + (tam_stack % tam_Bloque);

  *out = m_read(addr);

  procesos[actual_process_index].stack += 1;

  // En caso de terminar con el page_frame actual lo libera
  if (tam_stack % tam_Bloque == 0)
  {
    memoria_virtual[page_frame] = -1;
    procesos[actual_process_index].page_table[page] = -1;
    m_unset_owner(page_frame * tam_Bloque, (page_frame + 1) * tam_Bloque - 1);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  size_t actual_page = (size_t)(addr / tam_Bloque);

  if (memoria_virtual[actual_process_index] == actual_process_pid)
  {
    *out = m_read(addr);

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  size_t actual_page = (size_t)(addr / tam_Bloque);

  if (memoria_virtual[actual_page] == actual_process_pid)
  {
    m_write(addr, val);

    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  actual_process_pid = process.pid;

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (memoria_virtual[i] == process.pid)
    {
      actual_process_index = i;
      return;
    }
  }

  for (size_t i = 0; i < num_page_frames; i++)
  {
    if (!procesos[i].asignado)
    {
      procesos[i].asignado = 1;
      procesos[i].usuario = process.pid;
      memoria_virtual[i] = process.pid;
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
    if (procesos[i].usuario == process.pid)
    {
      procesos[i].asignado = 0;
      int page_frame;

      for (size_t j = 0; j < PAGES; j++)
      {
        page_frame = procesos[i].page_table[j];

        if (page_frame != -1)
        {
          memoria_virtual[page_frame] = 0;
        }

        procesos[i].page_table[j] = -1;
        m_unset_owner(i * tam_Bloque, (i + 1) * tam_Bloque - 1);
      }

      procesos[i].usuario = -1;
      procesos[i].heap = 0;
      procesos[i].stack = tam_Stack;
    }
  }
}