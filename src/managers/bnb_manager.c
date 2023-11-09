#include "bnb_manager.h"
#include "stdio.h"
#include "freelist.h"

#define STACK_SIZE 100
int bnb_bound;
typedef struct{
  /*
    base_physical = bnb_bound*i
    stack_base_virtual = code_size+STACK_SIZE
  */
  size_t code_size;
  size_t stack_pointer; //Apunta a la ultima posicion (virtual) ocupada del stack
  int owner;
  struct Freelist* freelist;
} BnB_Frame;
int bnb_frameCount;
BnB_Frame* bnb_frames = NULL; // Los trocitos de memoria fisica que cada proceso puede obtener

int bnb_current_process_index;

/*
    Se almacena en un frame:
      Codigo
      Espacio para stack
      Stack (que crece hacia direcciones menores)
      Heap
*/

///
///   Funciones auxiliares
///

int get_current_frame_base(){
  return bnb_current_process_index*bnb_bound;
}

///
///   Funciones de API
///

//// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  
  // Lista de BnB frames en memoria fisica
  bnb_bound = 1024;
  bnb_frameCount = m_size()/bnb_bound;

  free(bnb_frames);
  bnb_frames = malloc(bnb_frameCount*sizeof(BnB_Frame));

  for(int i = 0; i<bnb_frameCount; i++){
    bnb_frames[i].owner = NO_ONWER;
  }
}

//// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
//// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  BnB_Frame* frame = &(bnb_frames[bnb_current_process_index]);
  out->size = size;
  int rc = freelist_alloc_space(frame->freelist, size, &(out->addr));
  return rc;
}

//// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  BnB_Frame* frame = &(bnb_frames[bnb_current_process_index]);
  freelist_free_space(frame->freelist, ptr.addr, ptr.size);
  return 0;
}

//// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  // Header
  BnB_Frame* frame = &(bnb_frames[bnb_current_process_index]);
  size_t frame_base = get_current_frame_base();
  
  // Disminuir el SP y comprobar si hubo Stack Overflow
  frame->stack_pointer -= 1;
  if(frame->stack_pointer <= frame->code_size-1){
    return -1;
  }

  // Insertar el elemento
  m_write(frame_base + frame->stack_pointer, val);
  out->addr = frame->stack_pointer;
  out->size = 1;

  return 0;
}

//// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  
  // Leer lo ultimo del stack
  BnB_Frame* frame = &(bnb_frames[bnb_current_process_index]);
  size_t frame_base = get_current_frame_base();
  *out = m_read(frame_base + frame->stack_pointer);

  // Aumentar el SP y comprobar si hubo Stack Underflow
  frame->stack_pointer += 1;
  if(frame->stack_pointer  > frame->code_size+STACK_SIZE){
    return -1;
  }

  return 0;
}

//// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if(addr > (addr_t)bnb_bound-1){
    return -1;
  }

  *out = m_read(get_current_frame_base()+addr);
  return 0;
}

//// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  BnB_Frame* frame = &(bnb_frames[bnb_current_process_index]);
  
  if(addr > (addr_t)bnb_bound-1 || addr <= frame->code_size-1){
    return -1;
  }

  m_write(get_current_frame_base()+addr, val);
  return 0;
}

//// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  set_curr_owner(process.pid);

  // Buscar el trocito de memoria correspondiente al proceso
  int first_unowned_index = -1;
  for(int i = 0; i<bnb_frameCount; i++){
    if(process.pid == bnb_frames[i].owner){
      bnb_current_process_index = i;
      return;
    }
    if(first_unowned_index == -1 && bnb_frames[i].owner == NO_ONWER){
      first_unowned_index = i;
    }
  }

  // Si el proceso no tiene memoria, darle (de ser posible)
  if (first_unowned_index == -1){
    printf("Se ha acabado la memoria. There's nothing we can do.");
    exit(-1);
  }
  
  bnb_frames[first_unowned_index].owner = process.pid;
  bnb_frames[first_unowned_index].code_size = process.program->size;
  bnb_frames[first_unowned_index].stack_pointer = process.program->size+STACK_SIZE;
  bnb_frames[first_unowned_index].freelist = freelist_init(process.program->size+STACK_SIZE, bnb_bound-(process.program->size+STACK_SIZE));
  m_set_owner(first_unowned_index*bnb_bound, first_unowned_index*bnb_bound+bnb_bound-1);
  bnb_current_process_index = first_unowned_index;
}

//// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  for(int i = 0; i<bnb_frameCount; i++){
    if(process.pid == bnb_frames[i].owner){
      bnb_frames[i].owner = NO_ONWER;
      m_unset_owner(i*bnb_bound, i*bnb_bound+bnb_bound-1);
      freelist_deinit(bnb_frames[i].freelist);
      return;
    }
  }
}
