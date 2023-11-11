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
  // Libera la memoria existente, si la hay
  if (memoria_virtual != NULL) {
    free(memoria_virtual);
    memoria_virtual = NULL;
  }
  if (procesos != NULL) {
    free(procesos);
    procesos = NULL;
  }
  if (pfn != NULL) {
    free(pfn);
    pfn = NULL;
  }

  num_page_frames = KB(m_size());

  // Reserva memoria para los arreglos
  procesos = (info_proceso_t *)malloc(num_page_frames * sizeof(info_proceso_t));
  pfn = (int *)malloc(num_page_frames * sizeof(int));
  memoria_virtual = (int *)malloc(num_page_frames * sizeof(int));

  // Inicializa las variables para cada proceso
  for (size_t i = 0; i < num_page_frames; i++) {
    procesos[i].asignado = 0;
    procesos[i].page_table = (size_t *)malloc(PAGES * sizeof(size_t));

    for (size_t j = 0; j < PAGES; j++) {
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
    int found = 1; // Inicialmente, asumimos que no se encontró espacio.

    for (size_t i = 0; i < num_page_frames; i++)
    {
        // Comprueba si la página de memoria virtual está libre (-1).
        if (memoria_virtual[i] == -1)
        {
            found = 0; // Encontramos espacio disponible.

            // Establece la dirección de inicio y el tamaño del espacio reservado.
            out->addr = i * tam_Bloque;
            out->size = 1;

            // Marca la página de memoria como utilizada por el proceso actual.
            memoria_virtual[i] = actual_process_pid;

            // Actualiza la tabla de páginas del proceso actual.
            procesos[actual_process_index].page_table[0] = i;

            // Asigna propiedad del espacio reservado al proceso actual.
            m_set_owner(i * tam_Bloque, (i + 1) * tam_Bloque - 1);

            break; // Sal del bucle una vez que se haya reservado espacio.
        }
    }

    return found; // Devuelve 1 si no se encontró espacio, 0 si se encontró espacio.
}

// Función para liberar un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
    // Calculamos el número de marcos de página actual y final para el bloque de memoria.
    size_t actual_page_frame = ptr.addr / tam_Bloque;
    size_t end_page_frame = (ptr.addr + ptr.size) / tam_Bloque;

    // Verificamos si el bloque de memoria se encuentra dentro de los límites permitidos.
    if (end_page_frame >= num_page_frames || ptr.size > procesos[actual_process_index].heap)
    {
        return 1; // Retorna 1 para indicar un error.
    }

    // Verificamos si los marcos de página en el rango están asignados al proceso actual.
    for (size_t i = actual_page_frame; i < end_page_frame; i++)
    {
        if (memoria_virtual[i] != actual_process_pid)
        {
            return 1; // Retorna 1 si se encuentra un marco de página no asignado al proceso.
        }
    }

    // Actualizamos la cantidad de memoria heap disponible en el proceso.
    procesos[actual_process_index].heap -= ptr.size;

    // Liberamos los marcos de página en la tabla de páginas del proceso.
    for (size_t i = 0; i < PAGES; i++)
    {
        size_t page_frame = procesos[actual_process_index].page_table[i];

        if (page_frame > actual_page_frame && page_frame <= end_page_frame)
        {
            // Llamamos a la función para liberar el propietario del marco de página.
            m_unset_owner(page_frame * tam_Bloque, (page_frame + 1) * tam_Bloque - 1);

            // Marcamos la entrada en la tabla de páginas como no asignada.
            procesos[actual_process_index].page_table[i] = -1;
        }
    }

    return 0; // Retorna 0 para indicar éxito en la liberación de memoria.
}


int m_pag_push(byte val, ptr_t *out) {
  size_t tam_stack = tam_Stack - procesos[actual_process_index].stack;
  size_t page;

  if (procesos[actual_process_index].heap + 1 == procesos[actual_process_index].stack) {
    return 1;
  }

  // Agrega una nueva página a la memoria del programa actual si es necesario
  if (tam_stack % tam_Bloque == 0) {
    for (size_t i = 0; i < num_page_frames; i++) {
      if (memoria_virtual[i] == -1) {
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
int m_pag_pop(byte *out) {
  if (procesos[actual_process_index].stack == tam_Stack) {
    // El stack está lleno, no se puede quitar más elementos.
    return 1;
  }

  // Calcula la posición en la pila (stack) actual
  size_t tam_stack = tam_Stack - procesos[actual_process_index].stack;

  // Calcula la página a la que pertenece la dirección en la pila
  size_t page = PAGES - (size_t)(tam_stack / tam_Bloque) - 1;

  // Obtiene el número de marco de página para la página actual
  size_t page_frame = procesos[actual_process_index].page_table[page];

  // Calcula la dirección física
  size_t addr = (page_frame * tam_Bloque) + (tam_stack % tam_Bloque);

  // Lee el valor en la dirección física y lo almacena en 'out'
  *out = m_read(addr);

  // Incrementa el contador de elementos en la pila
  procesos[actual_process_index].stack += 1;

  // En caso de terminar con el marco de página actual, lo libera
  if (tam_stack % tam_Bloque == 0) {
    // Marca el marco de página como no utilizado en la memoria virtual
    memoria_virtual[page_frame] = -1;

    // Marca la entrada de la tabla de páginas como no válida
    procesos[actual_process_index].page_table[page] = -1;

    // Libera el marco de página en la memoria
    m_unset_owner(page_frame * tam_Bloque, (page_frame + 1) * tam_Bloque - 1);
  }

  // Operación exitosa
  return 0;
}

// Carga el valor en una dirección determinada
// Esta función carga una página de memoria virtual en la dirección 'addr' en un búfer 'out'.
// Devuelve 0 si la página está disponible en memoria virtual actual, de lo contrario, devuelve 1.
int m_pag_load(addr_t addr, byte *out) {
  // Calcula el número de página al que pertenece la dirección 'addr'.
  size_t actual_page = (size_t)(addr / tam_Bloque);

  // Comprueba si la página está en la memoria virtual del proceso actual.
  if (memoria_virtual[actual_process_index] == actual_process_pid) {
    // Si la página está en la memoria virtual, lee la página en el búfer 'out'.
    *out = m_read(addr);
    return 0; // Indica éxito (página cargada correctamente).
  }

  // La página no está en la memoria virtual del proceso actual.
  return 1; // Indica fallo (página no disponible en la memoria virtual).
}

// Almacena un valor en una dirección de página determinada
int m_pag_store(addr_t addr, byte val)
{
  // Calcula la página actual en función de la dirección y el tamaño del bloque
  size_t actual_page = (size_t)(addr / tam_Bloque);

  // Comprueba si la página actual está asignada al proceso actual
  if (memoria_virtual[actual_page] == actual_process_pid) {
    // Si la página está asignada, escribe el valor en la dirección
    m_write(addr, val);
    // Devuelve 0 para indicar que la operación fue exitosa
    return 0;
  }

  // Si la página no está asignada al proceso, devuelve 1 para indicar un error
  return 1;
}


// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
// Establece el proceso actual como el proceso proporcionado
  actual_process_pid = process.pid;

  // Busca una página en memoria virtual que corresponda al proceso actual
  for (size_t i = 0; i < num_page_frames; i++) {
    if (memoria_virtual[i] == process.pid) {
      // Si se encuentra una coincidencia, actualiza el índice del proceso actual y sale de la función
      actual_process_index = i;
      return;
    }
  }

  // Si no se encontró una página correspondiente al proceso actual, busca una página no asignada
  for (size_t i = 0; i < num_page_frames; i++) {
    if (!procesos[i].asignado) {
      // Si se encuentra una página no asignada, la asigna al proceso actual y actualiza el índice del proceso actual
      procesos[i].asignado = 1;
      procesos[i].usuario = process.pid;
      memoria_virtual[i] = process.pid;
      actual_process_index = i;
      break;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
    // Recorre todos los marcos de página
    for (size_t i = 0; i < num_page_frames; i++) {
        // Verifica si el proceso actual coincide con el propietario del marco de página
        if (procesos[i].usuario == process.pid) {
            // Desasigna el marco de página
            procesos[i].asignado = 0;
            int page_frame;

            // Recorre todas las páginas en la tabla de páginas del proceso
            for (size_t j = 0; j < PAGES; j++) {
                page_frame = procesos[i].page_table[j];

                // Si la página tiene un marco de página asignado, libérala
                if (page_frame != -1) {
                    memoria_virtual[page_frame] = 0;
                }

                // Marca la entrada de la tabla de páginas como no asignada (-1)
                procesos[i].page_table[j] = -1;

                // Llama a una función para liberar la propiedad del rango de direcciones
                m_unset_owner(i * tam_Bloque, (i + 1) * tam_Bloque - 1);
            }

            // Restablece las propiedades del proceso
            procesos[i].usuario = -1;
            procesos[i].heap = 0;
            procesos[i].stack = tam_Stack;
        }
    }
}

