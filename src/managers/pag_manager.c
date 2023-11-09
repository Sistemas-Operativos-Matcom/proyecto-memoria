#include "pag_manager.h"
#include "string.h"
#include "freelist.h"

#include "stdio.h"

#define PAGE_OFFSET_SIZE 8
int PAGE_SIZE  = 1<<PAGE_OFFSET_SIZE;
int pageFrameCount;
char* pageFramesOwned = NULL;


#define STACK_SIZE 100
typedef struct {
  /*
    base_physical = bnb_bound*i
    stack_base_virtual = code_size+STACK_SIZE
  */
  size_t code_size;
  size_t stack_pointer; //Apunta a la ultima posicion (virtual) ocupada del stack
  int owner;
  struct Freelist* freelist;
  int page_count;
  size_t* to_page_frame;

} Pag_Proc_Info;

int PAG_MAX_PROC_INFOS = 1<<13;
Pag_Proc_Info* pag_proc_infos = NULL;

int pag_current_process_index;

///
///   Funciones auxiliares
///

int append_free_pageframe(){
  
  for(int i = 0; i<pageFrameCount; i++){
    if (pageFramesOwned[i] == 0){
      Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
      pageFramesOwned[i] = 1;
      m_set_owner(i*PAGE_SIZE, i*PAGE_SIZE+PAGE_SIZE);
      info->to_page_frame[info->page_count] = i;
      info->page_count += 1;
      return 0;
    }
  }

  return -1;
}

int get_physical_address_of(int v_addr){
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);

  int virtual_page_number = v_addr >> PAGE_OFFSET_SIZE;
  int page_frame_number = info->to_page_frame[virtual_page_number];
  int page_frame_base = page_frame_number*PAGE_SIZE;
  int page_offset = v_addr & ((1<<PAGE_OFFSET_SIZE) - 1);

  return page_frame_base+page_offset;
}

///
///   Funciones de API
///

//// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  pageFrameCount = m_size()/PAGE_SIZE;
  free(pageFramesOwned);
  pageFramesOwned = (char*) malloc(sizeof(char)*pageFrameCount);
  free(pag_proc_infos);
  pag_proc_infos = (Pag_Proc_Info*) malloc(sizeof(Pag_Proc_Info)*PAG_MAX_PROC_INFOS);

  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    pag_proc_infos[i].owner = NO_ONWER;
  }
}

//// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
//// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
  out->size = size;
  
  // Mientras no quepa lo que se que se quiere alocar, pedir mas paginas...
  while(1){
    int rc = freelist_alloc_space(info->freelist, size, &(out->addr));
    if(rc != -1){ return 0; }

    int frame_rc = append_free_pageframe();
    if(frame_rc == -1){ return -1; }

    freelist_grow(info->freelist, PAGE_SIZE);
  }
}

//// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
  freelist_free_space(info->freelist, ptr.addr, ptr.size);
  printf("%d\n", debug_freelist_node_count(info->freelist));
  return 0;
}

//// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  // Header
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
  
  // Disminuir el SP y comprobar si hubo Stack Overflow
  info->stack_pointer -= 1;
  if(info->stack_pointer <= info->code_size-1){
    return -1;
  }

  // Insertar el elemento
  m_write(get_physical_address_of(info->stack_pointer), val);
  out->addr = info->stack_pointer;
  out->size = 1;

  return 0;
}

//// Quita un elemento del stack
int m_pag_pop(byte *out) {
  
  // Leer lo ultimo del stack
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
  *out = m_read(get_physical_address_of(info->stack_pointer));

  // Aumentar el SP y comprobar si hubo Stack Underflow
  info->stack_pointer += 1;
  if(info->stack_pointer > info->code_size+STACK_SIZE){
    return -1;
  }

  return 0;
}

//// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);

  if(addr > (addr_t)(info->page_count*PAGE_SIZE)-1){
    return -1;
  }

  *out = m_read(get_physical_address_of(addr));
  return 0;
}

//// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  Pag_Proc_Info* info = &(pag_proc_infos[pag_current_process_index]);
  
  if(addr > (addr_t)(info->page_count*PAGE_SIZE)-1 || addr <= info->code_size-1){
    return -1;
  }

  m_write(get_physical_address_of(addr), val);
  return 0;
}

//// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  set_curr_owner(process.pid);
  
  // Buscar la info. de la memoria del proceso
  int first_unowned_index = -1;
  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    if(pag_proc_infos[i].owner == process.pid){
      pag_current_process_index = i;
      return;
    }
    if(first_unowned_index == -1 && pag_proc_infos[i].owner == NO_ONWER){
      first_unowned_index = i;
    }
  }

  // Si no existe la info., crearla
  pag_current_process_index = first_unowned_index;
  int initial_page_count = (process.program->size+STACK_SIZE+10)/(PAGE_SIZE) + 1;

  Pag_Proc_Info* info = &pag_proc_infos[first_unowned_index];
  info->owner = process.pid;
  info->code_size = process.program->size;
  info->freelist = freelist_init(process.program->size+STACK_SIZE, initial_page_count*(PAGE_SIZE)-(process.program->size+STACK_SIZE));
  info->stack_pointer = process.program->size+STACK_SIZE;
  info->page_count = 0;
  info->to_page_frame = (size_t*) malloc(pageFrameCount*sizeof(size_t));
  memset(info->to_page_frame, -1, pageFrameCount*sizeof(size_t));

  for(int i = 0; i<initial_page_count; i++){
    int rc = append_free_pageframe();
    if(rc == -1){
      printf("Se ha acabado la memoria. There's nothing we can do.");
      exit(-1);
    }
  }
}

//// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    if (pag_proc_infos[i].owner == process.pid){
      
      freelist_deinit(pag_proc_infos[i].freelist);
      
      for(int j = 0; j<pageFrameCount; j++){
        int pageFrame = (int) pag_proc_infos[i].to_page_frame[j];
        if(pageFrame == -1){
          break;
        }
        
        pageFramesOwned[pageFrame] = 0;
        m_unset_owner(pageFrame*PAGE_SIZE, pageFrame*PAGE_SIZE+PAGE_SIZE);
      }

      free(pag_proc_infos[i].to_page_frame);
    }
  }
}
