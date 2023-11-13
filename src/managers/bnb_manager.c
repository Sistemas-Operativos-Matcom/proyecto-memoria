#include "bnb_manager.h"

#include "stdio.h"

static programSpaceData_t actual_process;
static space_t *spaces;
static addr_t actual_count;
static addr_t *addr_process;

#define default_space_size 1024

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //initialize values
  size_t cant_of_spaces = m_size() / default_space_size;

  if(spaces != NULL){
    free(spaces);
    spaces = NULL;
  }

  spaces = (space_t *)malloc(sizeof(space_t) *cant_of_spaces);
  addr_process = (addr_t *)malloc(sizeof(addr_t) * cant_of_spaces);
  actual_count = 0;

  if(addr_process != NULL){
    free(addr_process);
    addr_process = NULL;
  }

  size_t size_init = 0;
  
  for (size_t i = 0; i < cant_of_spaces; i++)
  {
    space_t* space_ = &spaces[i];
    space_->heap = size_init + 1;
    space_->stack = size_init + default_space_size - 1;
    space_->is_used = 0;

    space_->program.pid = 0;
    space_->program.base = size_init + 1;
    space_->program.bound = size_init + default_space_size - 1;
    space_->program.size = 0;
  }
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  for (size_t i = 0; i < m_size() / default_space_size; i++)
  {
    if(!spaces[i].is_used){
      size_t reserve = i* default_space_size;
      m_set_owner(reserve + 1, reserve + default_space_size - 1);

      actual_count = i;
      addr_process[actual_process.pid] = i;
      spaces[i].program.size = size;
      spaces[i].program.pid = actual_process.pid;
      spaces[i].is_used = 1;

      out->addr = reserve + 1;
      out->size = 1;
      return 0;
    }

  }

  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(ptr.addr >= spaces[actual_count].heap &&
     ptr.addr < spaces[actual_count].stack){
      m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
      spaces[actual_count].program.size = spaces[actual_count].program.size - ptr.size;
      return 0;
     }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(spaces[actual_count].program.bound - 1 > spaces[actual_count].program.base){
    m_write(spaces[actual_count].program.bound, val);
    out->addr = spaces[actual_count].program.bound;
    out->size = 1;
    spaces[actual_count].program.bound--;
    return 0;
  }
  else return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(spaces[actual_count].program.base == spaces[actual_count].program.bound) return 1;
  else{
    *out = m_read(spaces[actual_count].program.bound);
    spaces[actual_count].program.bound++;
  }

}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(addr >= spaces[actual_count].program.base &&
     addr < spaces[actual_count].program.bound){
      *out = m_read(addr);
      return 0;
     }
  else return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(addr >= spaces[actual_count].heap &&
     addr < spaces[actual_count].heap + spaces[actual_count].program.size){
      m_write(addr, val);
      return 0;
     }
  else return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  actual_process.pid = process.pid;
  actual_count = addr_process[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  m_unset_owner(spaces[addr_process[process.pid]].heap, 
                spaces[addr_process[process.pid]].stack);
  
  spaces[addr_process[process.pid]].program.base = spaces[addr_process[process.pid]].heap;  
  spaces[addr_process[process.pid]].program.bound = spaces[addr_process[process.pid]].stack;
  spaces[addr_process[process.pid]].is_used = 0;
  spaces[addr_process[process.pid]].program.pid = 0;
  spaces[addr_process[process.pid]].program.size = 0;
}
