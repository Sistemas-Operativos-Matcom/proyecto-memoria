#include "pag_manager.h"
#include "stdio.h"
#include "structs.h"

// Valores de paginación (MODIFICAR BAJO SU PROPIO CRITERIO)
static size_t Max_Process_Count = 16;
static size_t const Page_Size = 128;
static size_t const Max_Pages_in_Process = 64;
static size_t const Max_Address = Page_Size * Max_Pages_in_Process;

// Valores de memoria
static size_t Memory_Size; // Tamaño total de la memoria
static size_t VPN_Mask;    // Bits de selección de Physical Frame Number (PFN)
static size_t VPN_Shift;   // Bits de Offset

// Listas y variables universales
static List *Pages;                // Esta lista contiene las páginas
static Pagination *Processes;      // Este arreglo contiene la información de los procesos
static size_t current_process = 0; // Proceso actual
static int last_process = 0;       // Último proceso de la lista
static bool initialized = false;   // Indicador de inicialización
bool initialized_again = false;    // Indicador de reinicialización

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  Memory_Size = m_size();
  // Hallando logaritmo en base 2
  size_t offset_bits = -1;
  int n = Page_Size;
  while (n)
  {
    offset_bits++;
    n >>= 1;
  }
  // Asignando bits de Mask y de Offset
  VPN_Mask = LAST_ADDR ^ ((1 << offset_bits) - 1); // Operación XOR entre la representación binaria de offset y todos los bits en 1
  VPN_Shift = offset_bits;
  // Si ya la estructura estaba inicializada, limpiar y reiniciar
  if (initialized)
  {
    List_RS(Pages, Memory_Size / Page_Size);
    free(Processes);
    initialized_again = true;
  }
  // Iniciar estructura por primera vez
  else
  {
    Pages = (List *)malloc(sizeof(List));
    List_Init(Pages, Memory_Size / Page_Size);
  }
  // Asignando valores iniciales
  Processes = (Pagination *)malloc(Max_Process_Count * sizeof(Pagination));
  last_process = 0;
  current_process = 0;
  initialized = true;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  // Reservando un conjunto de páginas contiguas de tamaño suficiente
  size = size / Page_Size + (size_t)(size % Page_Size ? 1 : 0);
  size_t addr;
  // Obteniendo el fragmento de la memoria (página) con tamaño size disponible. En caso de error devuelve el error, sino guarda la dirección en addr
  if (Get_Mem(Pages, size, &addr))
  {
    return Error;
  }
  m_set_owner(addr * Page_Size, addr * Page_Size + size * Page_Size);    // Asignando fragmento de memoria disponible para un proceso
  size_t position = Find_Empty_Pages(&Processes[current_process], size); // Encontrar una posicion donde guardar los segmentos reservados
  out->addr = position * Page_Size;                                      // Guardar en out la direccion virtual donde estara la memoria

  // Guardando memoria reservada  (pueden ser varias páginas en dependencia del tamaño reequerido)
  for (size_t i = position; size; i++, size--)
  {
    Processes[current_process].virtual_page_number[i] = (addr + i); // Guardando VPN de la página
    Processes[current_process].is_page_valid[i] = true;             // Marcando página como disponible
  }
  return Ok;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  // Obteniendo primera página
  size_t first_page = (ptr.addr & VPN_Mask) >> VPN_Shift;// Operación AND entre los bits de la dirección del puntero y la máscara de dirección

  // Obteneniendo cantidad de páginas a liberar a partir de la primera
  size_t page_count = ptr.size / Page_Size + (size_t)(ptr.size % Page_Size ? 1 : 0);

  // Liberar la memoria
  if (Free_Mem(Pages, page_count, Processes[current_process].virtual_page_number[first_page]))
  {
    return Error;
  }

  // Designando fragmento de memoria no válido
  m_unset_owner(first_page * Page_Size, first_page * Page_Size + page_count * Page_Size);
  for (size_t i = first_page; page_count; i++, page_count--)
  {
    Processes[current_process].is_page_valid[i] = false;
  }
  return Ok;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  // Encontrando el stack pointer en la memoria física
  size_t stack_pointer = Processes[current_process].stack_pointer;
  stack_pointer = stack_pointer >= Max_Address ? Max_Address - 1 : stack_pointer;
  // Encontrando bits correspondientes
  size_t page = (stack_pointer & VPN_Mask) >> VPN_Shift;
  size_t offset = (stack_pointer & ~VPN_Mask) - 1;  //Negado VPN_Mask

  // Si la pagina no es válida, se reserva una
  if (!Processes[current_process].is_page_valid[page])
  {
    size_t temporal_addr;
    if (Get_Mem(Pages, 1, &temporal_addr))
    {
      return Error;
    }
    m_set_owner(temporal_addr * Page_Size, (temporal_addr + 1) * Page_Size);
    Processes[current_process].virtual_page_number[page] = temporal_addr;
    Processes[current_process].is_page_valid[page] = true;
  }

  // Calculando la dirección de memoria física
  size_t addr = Processes[current_process].virtual_page_number[page] * Page_Size + offset;
  // Guardando valor en memoria asignada
  m_write(addr, val);

  // Calcular el nuevo stack pointer y asignandolo al puntero out
  stack_pointer = (page << VPN_Shift) | offset;
  out->addr = stack_pointer;
  out->size = 1;
  Processes[current_process].stack_pointer = stack_pointer;
  return Ok;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  // Encontrando el stack pointer en la memoria física
  size_t stack_pointer = Processes[current_process].stack_pointer;
  // Encontrando bits correspondientes
  size_t page = (stack_pointer & VPN_Mask) >> VPN_Shift;
  size_t offset = stack_pointer & ~VPN_Mask;

  // Si el puntero sobrepasa el límite de la memoria o la página no está disponible, devuelve error
  if (stack_pointer >= Max_Address || !Processes[current_process].is_page_valid[page])
  {
    return Error;
  }

  // Calculando la dirección de memoria física
  size_t addr = Processes[current_process].virtual_page_number[page] * Page_Size + offset;
  //Asignando a out la lectura de la dirección de memoria
  *out = m_read(addr);

  // Actualizando el stack pointer
  Processes[current_process].stack_pointer = ++stack_pointer;

  // Si el puntero cambió de página
  if (page != (stack_pointer & VPN_Mask) >> VPN_Shift)
  {
    //Asignando la nueva dirección de página
    addr = Processes[current_process].virtual_page_number[page];

    // Liberando página (si falla devuelve error)
    if (Free_Mem(Pages, 1, addr))
    {
      return Error;
    }

    //Liberando espacio de página de la memoria
    m_unset_owner(addr * Page_Size, (addr + 1) * Page_Size);
  }

  // Retornar con exito
  return Ok;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  // Obteneniendo VPN
  size_t page = (addr & VPN_Mask) >> VPN_Shift;
  // Obteneniendo Offset
  size_t offset = addr & ~VPN_Mask;
  // Obtener la dirección de memoria física
  size_t p_addr = Processes[current_process].virtual_page_number[page] * Page_Size + offset;

  // Comprobando que la dirección no exceda los límites de la memoria y que la página sea válida
  if (p_addr > Max_Address || !Processes[current_process].is_page_valid[page])
  {
    return Error;
  }

  // Devolviendo respuesta
  *out = m_read(p_addr);
  return Ok;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  // Obteneniendo VPN
  size_t page = (addr & VPN_Mask) >> VPN_Shift;
  // Obteneniendo Offset
  size_t offset = ~VPN_Mask & (size_t)addr;
  // Obtener la dirección de memoria física
  size_t p_addr = Processes[current_process].virtual_page_number[page] * Page_Size + offset;

  // Comprobando que la dirección no exceda los límites de la memoria y que la página sea válida
  if (p_addr > Max_Address || !Processes[current_process].is_page_valid[page])
  {
    return Error;
  }

  // Escribir en memoria y devolver con éxito
  m_write(p_addr, val);
  return Ok;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  //Buscamos un espacio de memoria para el proceso, si ya lo tiene lo devolvemos
  int last_free = -1;
  for (int i = 0; i < last_process; i++)
  {
    if (Processes[i].process_id == process.pid)
    {
      current_process = i;
      return;
    }
    if (!Processes[i].is_process_working)
    {
      last_free = i;
    }
  }

  //Si no hay espacio de este proceso, buscamos el último espacio libre y se lo asignamos
  int position = last_free == -1 ? last_process++ : last_free;//Si no había ningún espacio libre entre procesos, se usa el espacio disponible después del último proceso

  //Iniciando una nueva tabla de paginación para el proceso nuevo
  Pag_Init(&Processes[position], process.pid, Max_Pages_in_Process);
  current_process = position;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  // Buscando el proceso
  for (int i = 0; i < last_process; i++)
  {
    if (Processes[i].process_id == process.pid)
    {
      // Marcando proceso como inactivo
      Processes[i].is_process_working = false;

      // Liberando memoria
      for (int l = 0, r; (size_t)l < Processes[i].total_memory_size; l++)
      {
        if (Processes[i].is_page_valid[l])
        {
          r = l;
          while (Processes[i].is_page_valid[++r]);
          Free_Mem(Pages, r - l, l);
          l = r;
        }
      }
      //Liberando espacio de VPN
      free(Processes[i].virtual_page_number);
      free(Processes[i].is_page_valid);
    }
  }
}
