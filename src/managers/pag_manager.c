#include "pag_manager.h"

#include "stdio.h"

#define pages_n 4
#define page_size 256
#define page_frame_amount m_size() / page_size

static int *memory;
static process_pag_t *processes;

static int this_process_pid;
static int this_process;





// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(memory != NULL){
    free(memory);
    memory = NULL;
  }

  if(processes != NULL){
    free(processes);
    processes = NULL;
  }

  memory = (int *)malloc(page_frame_amount * sizeof(int));
  processes = (process_pag_t *)malloc(page_frame_amount * sizeof(process_pag_t));
  
  for(size_t i = 0; i < page_frame_amount; i++){

    memory[i] = -1;
    processes[i].in_use = 0;
    processes[i].pid = -1;
    processes[i].heap_pointer = 0;
    processes[i].stack_pointer = pages_n * page_size - 1;

    processes[i].page_table = (size_t *)malloc(pages_n * sizeof(size_t));

    for(int j = 0; j < pages_n; j++){
      processes[i].page_table[j] = -1;
    }

  }





}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t heap_size = processes[this_process].heap_pointer + size;
  size_t this_page = (size_t)(heap_size / page_size);

  if(heap_size >= processes[this_process].stack_pointer){
    return 1;
  }

  processes[this_process].heap_pointer = heap_size;

    int safe = 1;

  for(size_t i = 0; i <= this_page; i++){
    for (size_t j = 0; j < page_frame_amount; j++){

      if(memory[j] == -1){
        
        memory[j] = this_process_pid;

        out->addr = j * page_size;
        out->size = 1;

        m_set_owner(j*page_size, j*page_size + page_size - 1);
        
        processes[this_process_pid].page_table[i] = j;
        return 0;

      }
    }
  }
  return safe;
}


// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t heap_size = processes[this_process].heap_pointer - 1;
  size_t new_heap_size = processes[this_process].heap_pointer - 1 - ptr.size;
  size_t this_page = (size_t)(heap_size / page_size);

  size_t ptr_page_f = (size_t)(ptr.addr / page_size);
  size_t ptr_page_l = (size_t)((ptr.addr + ptr.size) / page_size);

  size_t page_frame;

  if(ptr_page_l >= page_frame_amount || ptr.size > heap_size){
    return 1;
  }

  for(size_t i = ptr_page_f; i <= ptr_page_l; i++){
    if(memory[i] != this_process_pid){
      return 1;
    }
  }

  processes[this_process].heap_pointer = new_heap_size + 1;

  for(size_t i = 0; i < pages_n; i++){
    page_frame = processes[this_process].page_table[i];

    if(page_frame > ptr_page_f && page_frame <= ptr_page_l){
      m_unset_owner(page_frame * page_size, page_frame*page_size + page_size - 1);
      processes[this_process].page_table[i] = -1;
    }

  }
  
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);
  size_t process_stack = processes[this_process].stack_pointer;
  size_t stack_page = process_stack / page_size;
  size_t stack_addr;


  if(processes[this_process].heap_pointer + 1 >= process_stack){
    return 1;
  }
  
  if(processes[this_process].page_table[stack_page] == -1){
    for(size_t i = 0; i < page_frame_amount; i++){
      if(memory[i] == -1){
        memory[i] = this_process_pid;
        processes[this_process].page_table[stack_page] = i;
        m_set_owner(i * page_size, i *page_size + page_size - 1);
        break;
      }
    }
  } 

  stack_addr = (processes[this_process].page_table[stack_page] * page_size) + (process_stack % page_size);

  m_write(stack_addr, val);
  out->addr = stack_addr;
  

  processes[this_process].stack_pointer = process_stack - 1;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(processes[this_process].stack_pointer + 1 == pages_n * page_size){
    return 1;
  }

  size_t process_stack_out = processes[this_process].stack_pointer + 1;
  size_t stack_page_out = process_stack_out / page_size;
  size_t stack_page_frame = processes[this_process].page_table[stack_page_out];
  size_t stack_addr = (stack_page_frame * page_size) + (process_stack_out % page_size);
  size_t heap = processes[this_process].heap_pointer;
  processes[this_process].stack_pointer = process_stack_out;
  
  *out = m_read(stack_addr);

  if(process_stack_out % page_size == 0 && heap / page_size < stack_page_out){
    memory[stack_page_frame] = -1;
    processes[this_process].page_table[stack_page_out] = -1;
    m_unset_owner(stack_page_frame * page_size, stack_page_frame * page_size + page_size -1);
  }

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t this_page_frame = (size_t)(addr / page_size);

  if(memory[this_page_frame] == this_process_pid){
    *out = m_read(addr);
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t this_page_frame = (size_t)(addr / page_size);

  if(memory[this_page_frame] == this_process_pid){
    m_write(addr, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  this_process_pid = process.pid;

  

  for(size_t i = 0; i < page_frame_amount; i++){
    if(processes[i].pid == process.pid){
      this_process = i;
      return;
    }
  }

  for(size_t i = 0; i < page_frame_amount; i++){
    if(!processes[i].in_use){
      processes[i].in_use = 1;
      processes[i].pid = this_process_pid;
      this_process = i;
      return;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);
  int reset_page;

  for(size_t i = 0; i < page_frame_amount; i++){
    if(processes[i].pid == process.pid){
      processes[i].in_use = 0;
      processes[i].pid = -1;
      processes[i].heap_pointer = 0;
      processes[i].stack_pointer = pages_n * page_size - 1;


      for(size_t j = 0; j < pages_n; j++){
        reset_page = processes[i].page_table[j];

        if(reset_page != -1){

          memory[reset_page] = -1;
          processes[i].page_table[j] = -1;
          m_unset_owner(reset_page * page_size, reset_page * page_size + page_size - 1);
        }
      }
    }
  }








}
