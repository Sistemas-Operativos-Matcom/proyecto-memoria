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

  size_t code_size;
  size_t stack_pointer; 
  
  int owner;
  
  struct Freelist* freelist;
  
  int page_count;
  size_t* to_page_frame;

} Page;

int PAG_MAX_PROC_INFOS = 1<<13;
Page* pages = NULL;

int pag_current_process_index;

/*-- Funciones auxiliares--*/

int append_freePage(){
  
  for(int i = 0; i<pageFrameCount; i++){
    if (pageFramesOwned[i] == 0){
      Page* actualPage = &(pages[pag_current_process_index]);
      pageFramesOwned[i] = 1;
      m_set_owner(i*PAGE_SIZE, i*PAGE_SIZE+PAGE_SIZE);
      actualPage->to_page_frame[actualPage->page_count] = i;
      actualPage->page_count += 1;
      return 0;
    }
  }

  return -1;
}

int get_physicalAddres(int v_addr){
  Page* actualPage = &(pages[pag_current_process_index]);

  int virtual_page_number = v_addr >> PAGE_OFFSET_SIZE;
  int page_frame_number = actualPage->to_page_frame[virtual_page_number];
  int page_frame_base = page_frame_number*PAGE_SIZE;
  int page_offset = v_addr & ((1<<PAGE_OFFSET_SIZE) - 1);

  return page_frame_base+page_offset;
}


///   Funciones pag

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  pageFrameCount = m_size()/PAGE_SIZE;
  free(pageFramesOwned);
  pageFramesOwned = (char*) malloc(sizeof(char)*pageFrameCount);
  free(pages);
  pages = (Page*) malloc(sizeof(Page)*PAG_MAX_PROC_INFOS);

  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    pages[i].owner = NO_ONWER;
  }
}


int m_pag_malloc(size_t size, ptr_t *out) {
  Page* actualPage = &(pages[pag_current_process_index]);
  out->size = size;
  
  // Mientras no quepa lo que se que se quiere coger de memoria, solicitar paginas

  while(1){
    
    int rc = freelist_alloc_space(actualPage->freelist, size, &(out->addr));
    
    if(rc != -1){ return 0; }

    int page_rc = append_freePage();

    if(page_rc == -1){ return -1; }

    freelist_grow(actualPage->freelist, PAGE_SIZE);
  }
}

//// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  Page* actualPage = &(pages[pag_current_process_index]);
  freelist_free_space(actualPage->freelist, ptr.addr, ptr.size);
  printf("%d\n", debug_freelist_node_count(actualPage->freelist));
  return 0;
}

//// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  
  Page* actualPage = &(pages[pag_current_process_index]);
  
  //comprobar si se puede hacer
  actualPage->stack_pointer -= 1;
  if(actualPage->stack_pointer <= actualPage->code_size-1){
    return -1;
  }

  // Insertar el elemento
  m_write(get_physicalAddres(actualPage->stack_pointer), val);
  out->addr = actualPage->stack_pointer;
  out->size = 1;

  return 0;
}

//// Quita un elemento del stack
int m_pag_pop(byte *out) {
  
  // Leer lo ultimo del stack
  Page* actualPage = &(pages[pag_current_process_index]);
  *out = m_read(get_physicalAddres(actualPage->stack_pointer));

  // Aumentar el SP y comprobar si hubo Stack Underflow
  actualPage->stack_pointer += 1;
  
  if(actualPage->stack_pointer > actualPage->code_size+STACK_SIZE){
    return -1;
  }

  return 0;
}

//// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  Page* actualPage = &(pages[pag_current_process_index]);

  if(addr > (addr_t)(actualPage->page_count*PAGE_SIZE)-1){
    return -1;
  }

  *out = m_read(get_physicalAddres(addr));
  return 0;
}

//// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  Page* actualPage = &(pages[pag_current_process_index]);
  
  if(addr > (addr_t)(actualPage->page_count*PAGE_SIZE)-1 || addr <= actualPage->code_size-1){
    return -1;
  }

  m_write(get_physicalAddres(addr), val);
  return 0;
}

//// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  set_curr_owner(process.pid);
  
  // Buscar la actualPage. de la memoria del proceso
  int first_unowned_index = -1;
  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    if(pages[i].owner == process.pid){
      pag_current_process_index = i;
      return;
    }
    if(first_unowned_index == -1 && pages[i].owner == NO_ONWER){
      first_unowned_index = i;
    }
  }

  // Si no existe la actualPage., crearla
  pag_current_process_index = first_unowned_index;
  int initial_page_count = (process.program->size+STACK_SIZE+10)/(PAGE_SIZE) + 1;

  Page* actualPage = &pages[first_unowned_index];
  actualPage->owner = process.pid;
  actualPage->code_size = process.program->size;
  actualPage->freelist = freelist_init(process.program->size+STACK_SIZE, initial_page_count*(PAGE_SIZE)-(process.program->size+STACK_SIZE));
  actualPage->stack_pointer = process.program->size+STACK_SIZE;
  actualPage->page_count = 0;
  actualPage->to_page_frame = (size_t*) malloc(pageFrameCount*sizeof(size_t));
  memset(actualPage->to_page_frame, -1, pageFrameCount*sizeof(size_t));

  for(int i = 0; i<initial_page_count; i++){
    int rc = append_freePage();
    if(rc == -1){
      printf("Se ha acabado la memoria. There's nothing we can do.");
      exit(-1);
    }
  }
}

//// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for(int i = 0; i<PAG_MAX_PROC_INFOS; i++){
    if (pages[i].owner == process.pid){
      
      freelist_deinit(pages[i].freelist);
      
      for(int j = 0; j<pageFrameCount; j++){
        int pageFrame = (int) pages[i].to_page_frame[j];
        if(pageFrame == -1){
          break;
        }
        
        pageFramesOwned[pageFrame] = 0;
        m_unset_owner(pageFrame*PAGE_SIZE, pageFrame*PAGE_SIZE+PAGE_SIZE);
      }

      free(pages[i].to_page_frame);
    }
  }
}
