#include "pag_manager.h"

#include "stdio.h"

static int actual_process_pid;
static addr_t actual_count;
//static size_t pages_frames;
static process_info_t *processes;
static addr_t *spaces;
static addr_t *phy_frame_numbers;
static addr_t pages_frames;

#define pages 4
#define default_page_size 64

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(processes != NULL){
    free(processes);
    processes = NULL;
  }

  if(spaces != NULL){
    free(spaces);
    spaces = NULL;
  }

  if(phy_frame_numbers != NULL){
    free(phy_frame_numbers);
    phy_frame_numbers = NULL;
  }

  pages_frames = m_size() / default_page_size;

  processes = (process_info_t *)malloc(pages_frames * sizeof(process_info_t));
  spaces = (addr_t *)malloc(pages_frames * sizeof(addr_t));
  phy_frame_numbers = (addr_t *)malloc(pages_frames * sizeof(addr_t));

  for (size_t i = 0; i < pages_frames; i++)
  {
    processes[i].pid = -1;
    processes[i].page_table = (addr_t *)malloc(pages * sizeof(addr_t));
    processes[i].is_used = 0;
    for (size_t j = 0; j < pages; j++)
    {
      processes[i].page_table[j] = -1;
    }
    processes[i].heap = 0;
    processes[i].stack = pages * default_page_size;
    phy_frame_numbers[i] = i;
  }
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  int exist = 1;

  for (size_t i = 0; i < pages_frames; i++)
  {
    if(spaces[i] == -1){
      out->addr = i * default_page_size;
      out->size = 1;
      processes[actual_count].page_table[0] = i;
      spaces[i] = actual_process_pid;
      m_set_owner(i * default_page_size, (i + 1) * default_page_size - 1);
      exist = 0;
      break;
    }
  }
  
  return exist;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t initial_page_frame = ptr.addr / default_page_size;
  size_t end_page_frame = (ptr.addr + ptr.size) / default_page_size;

  if(processes[actual_count].heap < ptr.size) return 1;
  if(end_page_frame >= pages_frames) return 1;
  for (size_t i = initial_page_frame; i < end_page_frame; i++)
  {
    if(spaces[i] != actual_process_pid) return 1;
  }

  processes[actual_count].heap = processes[actual_count].heap - ptr.size;
  
  for (size_t i = 0; i < pages; i++)
  {
    size_t page_frame = processes[actual_count].page_table[i];

    if(page_frame <= end_page_frame && initial_page_frame < page_frame){
      m_unset_owner(page_frame * default_page_size, (page_frame + 1) * default_page_size - 1);
      processes[initial_page_frame].page_table[i] = -1;
    }
  }
  
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(processes[actual_count].heap + 1 == processes[actual_count].stack) return 1;

  size_t stack_size = pages * default_page_size - processes[actual_count].stack;
  size_t page;

  if(stack_size % default_page_size == 0){
    for (size_t i = 0; i < pages_frames; i++)
    {
      if(spaces[i] == -1){
        spaces[i] = actual_process_pid;
        page = pages - stack_size / default_page_size - 1;
        processes[actual_count].page_table[page] = i;
        m_set_owner(i * default_page_size, (i + 1) * default_page_size - 1);
        break;
      }
    }
  }

  processes[actual_count].stack -= 1;
  stack_size += 1;
  page = pages - stack_size / default_page_size - 1;
  size_t page_frame = processes[actual_count].page_table[page];
  m_write(page_frame * default_page_size + stack_size * default_page_size, val);
  out->addr = processes[actual_count].stack;

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(processes[actual_count].stack == pages * default_page_size) return 1;

  size_t stack_size = pages * default_page_size - processes[actual_count].stack;
  size_t page = pages - stack_size / default_page_size - 1;
  size_t page_frame = processes[actual_count].page_table[page];

  *out = m_read(page_frame * default_page_size + stack_size * default_page_size);
  processes[actual_count].stack += 1;

  if(stack_size % default_page_size == 0){
    spaces[page_frame] = -1;
    processes[actual_count].page_table[page] = -1;
    m_unset_owner(page_frame * default_page_size, (page_frame + 1) * default_page_size - 1);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(spaces[addr / default_page_size] == actual_process_pid){
    *out = m_read(addr);
    return 0;
  }
  else return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(spaces[addr / default_page_size] == actual_process_pid){
  m_write(addr, val);
  return 0;
  }
  else return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  actual_process_pid = process.pid;

  for (size_t i = 0; i < pages_frames; i++)
  {
    if(spaces[i] == process.pid){
      actual_count = i;
      return;
    }
  }

  for (size_t i = 0; i < pages_frames; i++)
  {
    if(!processes[i].is_used){
      processes[i].is_used = 1;
      processes[i].pid = process.pid;
      spaces[i] = process.pid;
      actual_count = i;
      break;
    }
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t pages_frames = m_size() / default_page_size;

  for (size_t i = 0; i < pages_frames; i++)
  {
    if(processes[i].pid == process.pid){
      processes[i].is_used = 0;

      size_t page_frame;

      for (size_t j = 0; j < pages; j++)
      {
        page_frame = processes[i].page_table[j];
        if(page_frame != -1){
          spaces[page_frame] = 0;
        }

        processes[i].page_table[j] = -1;
        m_unset_owner(i * default_page_size, (i + 1) * default_page_size - 1);
      }

      processes[i].pid = -1;
      processes[i].heap = 0;
      processes[i].stack = pages * default_page_size;
    }
  }
  
}
