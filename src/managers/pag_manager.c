#include "pag_manager.h"
#include "stdio.h"

typedef struct page{
    int active;
    int use;
    size_t *page;
    size_t heap;
    size_t stack;
} process_memory_page_t;

int PageSize = 16;
int processAmount = 32;
int pagePerProcess;
int pag_pid;
int in;
int pageAmount;
int *mem;
int *page;
process_memory_page_t *pageMem;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  free(mem);
  free(page);
  free(pageMem);
  pageAmount = m_size()/PageSize;
  pagePerProcess = pageAmount/processAmount;
  mem = (int *)malloc(pageAmount * sizeof(process_memory_page_t));
  page = (int *)malloc(pageAmount * sizeof(pageAmount));
  pageMem = (process_memory_page_t *)malloc(pageAmount*sizeof(process_memory_page_t));

  for(int i = 0; i < pageAmount; i++){
    pageMem[i].active = 0;
    pageMem[i].page = (size_t*)malloc (pagePerProcess*sizeof(size_t));
    pageMem[i].use = -1;
    pageMem[i].heap = 0;
    pageMem[i].stack = pagePerProcess * PageSize;
    mem[i] = -1;
    page[i] = i;

    for(int j = 0; j < pagePerProcess; j++){
      pageMem[i].page[j] = -1;
    }

  }

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  for(int i = 0; i <  pageAmount; i++){
    if(mem[i] == -1){
      out->addr = (i*PageSize);
      out->size = 1;
      mem[i] = pag_pid;
      pageMem[in].page[0]=i;
      m_set_owner(i*PageSize, (i+1) * PageSize - 1);
      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  if((int)((ptr.addr + ptr.size) / PageSize) >= pageAmount || ptr.size > pageMem[in].heap)
  return 1;

  for(int i = ptr.addr/PageSize; i<(int)((ptr.addr +ptr.size)/PageSize); i++){
    if(mem[i] != pag_pid){
      return 1;
    }
  }

  pageMem[in].heap -= ptr.size;

  for(int i = 0; i < pagePerProcess; i++){
    if(pageMem[in].page[i] > ptr.addr/PageSize && pageMem[in].page[i] <= (ptr.addr+ptr.size)/PageSize){
      m_unset_owner(pageMem[in].page[i] * PageSize, (pageMem[in].page[i] + 1)*PageSize-1);
      pageMem[ptr.addr/PageSize].page[i] = -1;
    }
  }

  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if(pageMem[in].heap + 1 == pageMem[in].stack)return 1;
  int currentSize = pagePerProcess * PageSize - pageMem[in].stack;
  
  if(currentSize%PageSize == 0){

    for(int i = 0; i < pageAmount; i++){

      if(mem[i] == -1){

        mem[i] = pag_pid;
        pageMem[in].page[pagePerProcess-(currentSize/PageSize)-1] = i;
        m_set_owner(i*PageSize, (i+1) * PageSize - 1);
        break;
      }
    }
  }

  pageMem[in].stack -= 1;
  currentSize += 1;
  m_write((pageMem[in].page[pagePerProcess-(currentSize/PageSize)-1] * PageSize)+ (currentSize%PageSize), val);
  out->addr = pageMem[in].stack;
  return 0;
  }

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if((int)pageMem[in].stack == pagePerProcess * PageSize)return 1;

  int currentSize = pagePerProcess*PageSize - pageMem[in].stack;
  *out = m_read((pageMem[in].page[pagePerProcess- currentSize/PageSize -1]));
  pageMem[in].stack += 1;

  if(currentSize%PageSize == 0){
    mem[pageMem[in].page[pagePerProcess- currentSize/PageSize -1]] = -1;
    pageMem[in].page[pagePerProcess- currentSize/PageSize -1] = -1;
    m_unset_owner(pageMem[in].page[pagePerProcess- currentSize/PageSize -1] * PageSize, (pageMem[in].page[pagePerProcess- currentSize/PageSize -1]) * PageSize -1);
  }
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  if(mem[in] == pag_pid){
    *out = m_read(addr);
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  if(mem[(addr/PageSize)] == pag_pid){
    m_write(addr, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  pag_pid = process.pid;
  for(int i = 0; i < pageAmount; i++){
    if(mem[i] == process.pid){
      in = i;
      return;
    }
  }
  for(int i = 0; i < pageAmount; i++){
    if(!pageMem[i].active){
        pageMem[i].active = 1;
        pageMem[i].use = process.pid;
        mem[i] = process.pid;
        in = i;
        break;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for(int i = 0; i <  pageAmount; i++){
    if(pageMem[i].use == process.pid){
      pageMem[i].active = 0;
      pageMem[i].use = -1;
      pageMem[i].heap = 0;
      pageMem[i].stack = pagePerProcess * PageSize;
      for(int j = 0; j < pagePerProcess; j++){
        if((int)pageMem[i].page[j] != -1){
          mem[pageMem[i].page[j]] = 0;
          pageMem[i].page[j] = -1;
          m_unset_owner(i*PageSize, (i + 1)*PageSize -1);
        }
        
      }
    }
  }
}
