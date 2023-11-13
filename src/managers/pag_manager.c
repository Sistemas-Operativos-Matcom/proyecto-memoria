#include "pag_manager.h"
#include "stdio.h"
#include "free_list.h"

static size_t MAX_PROC_COUNT = 8;
static size_t const page_size = 64;
static size_t const MAX_PAGES_IN_PROC = 32;
static size_t const MAX_ADDR = page_size * MAX_PAGES_IN_PROC;

static size_t memory_size;
static size_t VPN_SHIFT;
static size_t offset_MASK;

static free_list_t *free_page_frames;
static virtual_process_t *procs;
static size_t curr_proc = 0;
static int last_proc = 0;
static int is_init = 0;
static int init_again = 0;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  memory_size = m_size();

  //Guardar la cantidad de bits del offset
  VPN_SHIFT = Get_Log2(page_size);
  //Crear una máscara para quedarse con los bits del offset
  offset_MASK = (1 << VPN_SHIFT) - 1;

  if (is_init)
  {
    Reset_free_list(free_page_frames, memory_size / page_size);
    free(procs);
    init_again = 1;
  }
  else
  {
    free_page_frames = (free_list_t *)malloc(sizeof(free_list_t));
    Init_free_list(free_page_frames, memory_size / page_size);
  }

  procs = (virtual_process_t *)malloc(MAX_PROC_COUNT * sizeof(virtual_process_t));
  last_proc = 0;
  curr_proc = 0;

  for (size_t i = 0; i < MAX_PROC_COUNT; i++)
  {
    procs[i].pid = -1;
  }

  is_init = 1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  // Guardar cuántas páginas son necesarias para reservar un espacio de tamaño size
  size = size / page_size + (size_t)(size % page_size ? 1 : 0);
  size_t addr;

  //Guardar en addr el espacio la página de la memoria física a partir de la cual se va a reservar el espacio
  if (Get_from_memory(free_page_frames, size, &addr))
    return MEM_FAIL;

  m_set_owner(addr * page_size, addr * page_size + size * page_size);

  // Encontrar un espacio en la meoria virtual donde guardar los segmentos reservados
  size_t first_virtual_page = Find_pages(&procs[curr_proc], size);

  // Guardar en out la direccion virtual donde estara la memoria
  out->addr = first_virtual_page * page_size;
  out->size = size * page_size;

  // Guardar las paginas que se reservaron
  for (size_t i = first_virtual_page; size; i++, size--)
  {
    procs[curr_proc].pfn[i] = (addr + i - first_virtual_page); 
    procs[curr_proc].page_valid[i] = 1;
  }
  return MEM_SUCCESS;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  // Obtener el numero de pagina virtual
  size_t first_page = ptr.addr>> VPN_SHIFT;
  // Obtener el número de páginas a liberar
  size_t page_count = ptr.size / page_size + (size_t)(ptr.size % page_size ? 1 : 0);

  // Liberar la memoria física
  if (Free_memory(free_page_frames, page_count, procs[curr_proc].pfn[first_page]))
    return MEM_FAIL;

  m_unset_owner(procs[curr_proc].pfn[first_page] * page_size, procs[curr_proc].pfn[first_page] * page_size + page_count * page_size);

  // Marcar la memoria virtual como no valida
  for (size_t i = first_page; page_count; i++, page_count--)
  {
    procs[curr_proc].page_valid[i] = 0;
  }

  return MEM_SUCCESS;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  // Encontrar el stack point fisico
  size_t sp = procs[curr_proc].stack_point;
  sp = sp >= MAX_ADDR ? MAX_ADDR - 1 : sp;
  //size_t page = (sp & VPN_MASK) >> VPN_SHIFT;
  size_t page = sp >> VPN_SHIFT;
  size_t offset = (sp & offset_MASK)-1;

  if (!procs[curr_proc].page_valid[page])
  {
    size_t tmp;

    if (Get_from_memory(free_page_frames, 1, &tmp))
      return MEM_FAIL;

    m_set_owner(tmp * page_size, (tmp + 1) * page_size);

    procs[curr_proc].pfn[page] = tmp;
    procs[curr_proc].page_valid[page] = 1;
  }

  // Calcular la direccion fisica
  size_t addr = procs[curr_proc].pfn[page] * page_size + offset;

  // Guardar el valor ahi
  m_write(addr, val);

  // Calcular el nuevo stack point
  sp = (page << VPN_SHIFT) | offset;

  // Devolver el valor en el puntero
  out->addr = sp;
  out->size = 1;

  // Actualizar el stack point
  procs[curr_proc].stack_point = sp;

  // Retornar con exito
  return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  // Encontrar el stack point fisico
  size_t sp = procs[curr_proc].stack_point;
  //size_t page = (sp & VPN_MASK) >> VPN_SHIFT;
  size_t page = sp >> VPN_SHIFT;
  size_t offset = sp & offset_MASK;

  // Chequear si la accion es valida
  if (sp >= MAX_ADDR || !procs[curr_proc].page_valid[page])
    return MEM_FAIL;

  // Calcular la direccion fisica
  size_t addr = procs[curr_proc].pfn[page] * page_size + offset;

  // Guardar en out el resultado
  *out = m_read(addr);

  // Actualizar el stack point
  procs[curr_proc].stack_point = ++sp;

  // Si se sale de una pagina
  if (page != sp >> VPN_SHIFT)
  {
    addr = procs[curr_proc].pfn[page];

    // Liberar esa pagina
    if (Free_memory(free_page_frames, 1, addr))
      return MEM_FAIL;

    m_unset_owner(addr * page_size, (addr + 1) * page_size);
  }

  // Retornar con exito
  return MEM_SUCCESS;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  // Obtener el numero de pagina virtual
  size_t page = addr>> VPN_SHIFT;
  // Obtener el offset
  size_t offset = addr & offset_MASK;

  // Obtener la direccion fisica
  size_t _addr = procs[curr_proc].pfn[page] * page_size + offset;

  // Chequear que sea valida la operacion
  if (_addr > MAX_ADDR || !procs[curr_proc].page_valid[page])
    return MEM_FAIL;

  // Retornar con exito
  *out = m_read(_addr);
  return MEM_SUCCESS;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  // Obtener el numero de pagina virtual
  size_t page = addr >> VPN_SHIFT;
  // Obtener el offset
  size_t offset = offset_MASK & (size_t) addr;

  // Obtener la direccion fisica
  size_t _addr = procs[curr_proc].pfn[page] * page_size + offset;

  // Chequear que sea valida la operacion
  if (_addr > MAX_ADDR || !procs[curr_proc].page_valid[page])
    return MEM_FAIL;

  m_write(_addr, val);

  // Retornar con exito
  return MEM_SUCCESS;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  int last_free = -1;
  // Buscar un espacio para el proceso, si ya esta solo se devuelve
  for (int i = 0; i < last_proc; i++)
  {
    if (procs[i].pid == process.pid)
    {
      curr_proc = i;
      return;
    }
    if (!procs[i].active)
    {
      last_free = i;
    }
  }

  // En caso de que el proceso sea nuevo, se añade en la primera
  // posicion donde no halla ninguno
  int pos = last_free == -1 ? last_proc++ : last_free;

  Init_virtual_process(&procs[pos], process.pid, MAX_PAGES_IN_PROC);

  curr_proc = pos;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  // Encontrar el proceso
  for (int i = 0; i < last_proc; i++)
  {
    if (procs[i].pid == process.pid)
    {
      // Marcarlo como no activo
      procs[i].active = 0;

      // Liberar la memoria que ocupa
      for (int l = 0, r; (size_t)l < procs[i].total_mem; l++)
      {
        if (procs[i].page_valid[l])
        {
          r = l;
          while (procs[i].page_valid[++r])
            ;
          Free_memory(free_page_frames, r - l, l);
          l = r;
        }
      }

      free(procs[i].pfn);
      free(procs[i].page_valid);
    }
  }
}
