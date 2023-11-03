#include "pag_manager.h"

#include "stdio.h"


// Almacena la informacion para cada proceso
static page_table_entry *linear_page_table;

// Crea el espacio de memoria virtual
static int *virtual_memory;

// Contiene los PFN de cada page_frame
static int *pfn;

// Contiene el ID del proceso actual
static int actual_process_pid;

// Contiene el index del arreglo de procesos del proceso actual
static int actual_process_index;

// Contiene la cantidad total de page_frames
static size_t num_page_frames;

#define PAGES 4
#define BLOCK_SIZE 256
#define CODE_SIZE 1
#define STACK_SIZE (PAGES * BLOCK_SIZE)
#define TO_KB(size) size / BLOCK_SIZE


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  /* Libero la memoria en caso de estar reservadas previamente */
  if (virtual_memory != NULL) {
    free(virtual_memory);
    virtual_memory = NULL;
  }
  if (linear_page_table != NULL) {
    free(linear_page_table);
    linear_page_table = NULL;
  }
  if (pfn != NULL) {
    free(pfn);
    pfn = NULL;
  }

  num_page_frames = TO_KB(m_size());
  
  linear_page_table = (page_table_entry*)malloc(num_page_frames * sizeof(page_table_entry));
  pfn = (int*)malloc(num_page_frames * sizeof(num_page_frames));
  virtual_memory = (int*)malloc(num_page_frames * sizeof(int));

  for (size_t i = 0; i < num_page_frames; i++) {
    // inicializa las variables para cada proceso
    linear_page_table[i].is_allocated = 0;
    linear_page_table[i].page_table = (size_t*)malloc(PAGES * sizeof(size_t));

    for (size_t j = 0; j < PAGES; j++) {
      linear_page_table[i].page_table[j] = -1;
    }
    
    linear_page_table[i].owner = -1;

    linear_page_table[i].heap = 0;
    linear_page_table[i].stack = STACK_SIZE;

    // Marca el page_frame como no usado
    virtual_memory[i] = -1;

    // Asigna los PNF correspondientes a cada page_frame
    pfn[i] = i;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int found = 1;

  for(size_t i = 0; i < num_page_frames; i++) {
    if (virtual_memory[i] == -1) {
      found = 0;

      out->addr = (i * BLOCK_SIZE);
      out->size = 1;

      virtual_memory[i] = actual_process_pid;
      linear_page_table[actual_process_index].page_table[0] = i;
      m_set_owner(i * BLOCK_SIZE, (i + 1) * BLOCK_SIZE - 1);

      break;
    }
  }

  return found;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  size_t actual_page_frame = (size_t)(ptr.addr / BLOCK_SIZE);
  size_t end_page_frame = (size_t)((ptr.addr + ptr.size) / BLOCK_SIZE);
  int found = 0;

  if (end_page_frame >= num_page_frames || ptr.size > linear_page_table[actual_process_index].heap) {
    return 1;
  }

  for (size_t i = actual_page_frame; i < end_page_frame; i++)
  {
    if (virtual_memory[i] != actual_process_pid) {
      found = 1;
      break;
    }
  }
  
  if (found) {
    return 1;
  }
  
  linear_page_table[actual_process_index].heap -= ptr.size;
  size_t page_frame;

  for (size_t i = 0; i < PAGES; i++) {
    page_frame = linear_page_table[actual_process_index].page_table[i];

    if (page_frame > actual_page_frame && page_frame <= end_page_frame) {
      m_unset_owner(page_frame * BLOCK_SIZE, (page_frame + 1) * BLOCK_SIZE - 1);
      linear_page_table[actual_page_frame].page_table[i] = -1;
    }
  }

  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {  
  size_t stack_size = STACK_SIZE - linear_page_table[actual_process_index].stack;
  size_t page;

  if (linear_page_table[actual_process_index].heap + 1 == linear_page_table[actual_process_index].stack) {
    return 1;
  }

  // Agrega una nueva pagina a la memoria del programa actual
  if (stack_size % BLOCK_SIZE == 0) {
    for (size_t i = 0; i < num_page_frames; i++) {
      if (virtual_memory[i] == -1) {
        virtual_memory[i] = actual_process_pid;

        page = PAGES - (size_t)(stack_size / BLOCK_SIZE) - 1;
        linear_page_table[actual_process_index].page_table[page] = i;
        m_set_owner(i * BLOCK_SIZE, (i + 1) * BLOCK_SIZE - 1);

        break;
      }
    }
  }

  // Agrega un nuevo elemento al stack
  linear_page_table[actual_process_index].stack -= 1;
  stack_size += 1;
  page = PAGES - (size_t)(stack_size / BLOCK_SIZE) - 1;
  size_t page_frame = linear_page_table[actual_process_index].page_table[page];
  size_t addr = (page_frame * BLOCK_SIZE) + (stack_size % BLOCK_SIZE);

  m_write(addr, val);

  out->addr = linear_page_table[actual_process_index].stack;

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if (linear_page_table[actual_process_index].stack == STACK_SIZE) {
    return 1;
  }

  // Encuentra la direccion fisica
  size_t stack_size = STACK_SIZE - linear_page_table[actual_process_index].stack;
  size_t page = PAGES - (size_t)(stack_size / BLOCK_SIZE) - 1;
  size_t page_frame = linear_page_table[actual_process_index].page_table[page];
  size_t addr = (page_frame * BLOCK_SIZE) + (stack_size % BLOCK_SIZE);
  
  *out = m_read(addr);

  linear_page_table[actual_process_index].stack += 1;

  // En caso de terminar con el page_frame actual lo libera
  if (stack_size % BLOCK_SIZE == 0) {
    virtual_memory[page_frame] = -1;
    linear_page_table[actual_process_index].page_table[page] = -1;
    m_unset_owner(page_frame * BLOCK_SIZE, (page_frame + 1) * BLOCK_SIZE - 1);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  size_t actual_page = (size_t)(addr / BLOCK_SIZE);

  if (virtual_memory[actual_process_index] == actual_process_pid) {
    *out = m_read(addr);

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  size_t actual_page = (size_t)(addr / BLOCK_SIZE);

  if (virtual_memory[actual_page] == actual_process_pid) {
    m_write(addr, val);

    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  actual_process_pid = process.pid;

  for (size_t i = 0; i < num_page_frames; i++) {
    if (virtual_memory[i] == process.pid) {
      actual_process_index = i;
      return;
    }
  }

  for (size_t i = 0; i < num_page_frames; i++) {
    if (!linear_page_table[i].is_allocated) {
      linear_page_table[i].is_allocated = 1;
      linear_page_table[i].owner = process.pid;
      virtual_memory[i] = process.pid;
      actual_process_index = i;
      
      break;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for (size_t i = 0; i < num_page_frames; i++) {
    if (linear_page_table[i].owner == process.pid) {
      linear_page_table[i].is_allocated = 0;
      int page_frame;

      for (size_t j = 0; j < PAGES; j++)
      {
        page_frame = linear_page_table[i].page_table[j];

        if (page_frame != -1) {
          virtual_memory[page_frame] = 0;
        }

        linear_page_table[i].page_table[j] = -1;
        m_unset_owner(i * BLOCK_SIZE, (i + 1) * BLOCK_SIZE - 1);
      }
      
      linear_page_table[i].owner = -1;
      linear_page_table[i].heap = 0;
      linear_page_table[i].stack = STACK_SIZE;
    }
  }
}
