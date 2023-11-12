#include "bnb_manager.h"
#include "stdio.h"

#define blockSize 1024
#define codeSize 1
#define Kb(size) ((size)/blockSize)

static addr_t *procsAddr;
static Bloque_t *virtualMem;
static addr_t curAddr;
static int curPID;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv){
  if(virtualMem != NULL){
    // libera la memoria
    free(virtualMem);
    virtualMem = NULL;
  }
  if(procsAddr != NULL){
    // libera las direcciones de procesos
    free(procsAddr);
    procsAddr = NULL;
  }

  size_t nBlocks = Kb(m_size());
  virtualMem = (Bloque_t *)malloc(sizeof(Bloque_t) * nBlocks);
  procsAddr = (size_t *)malloc(sizeof(size_t) * nBlocks);
  curAddr = 0;

  // inicializa cada bloque
  for(size_t i = 0, start = 0; i < nBlocks; i++, start += blockSize){
    virtualMem[i].heap = start + codeSize;
    virtualMem[i].stack = start + blockSize - 1;
    virtualMem[i].start = start + codeSize;
    virtualMem[i].end = start + blockSize - 1;
    virtualMem[i].size = 0;
    virtualMem[i].onUse = 0;
    virtualMem[i].owner = NO_OWNER;
  }
}


// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out){
  for(size_t i = 0; i < Kb(m_size()); i++){
    if(!virtualMem[i].onUse){
      size_t shift = i * blockSize;
      m_set_owner(shift + codeSize, shift + blockSize - 1);
      
      procsAddr[curPID] = i;
      curAddr = i;
      virtualMem[i].onUse = 1;  
      virtualMem[i].owner = curPID;  
      virtualMem[i].size = size;  

      // output
      out->addr = shift + codeSize;  
      out->size = 1;
      return 0;  
    }
  }
  return 1; 
}


// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr){
  size_t start = virtualMem[curAddr].start, end = virtualMem[curAddr].end;
  
  // si la dirección pertenece al bloque actual.
  if(ptr.addr >= start && ptr.addr + ptr.size < end){
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);  
    virtualMem[curAddr].size -= ptr.size;  
    return 0; 
  }
  return 1;  
}


// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out){
  // si stack está lleno
  if(virtualMem[curAddr].stack - 1 <= virtualMem[curAddr].heap){
    return 1;
  }

  // agrega el dato al stack y cambia la direccion
  m_write(virtualMem[curAddr].stack, val);
  virtualMem[curAddr].stack--;
  out->addr = virtualMem[curAddr].stack;
  return 0;
}


// Quita un elemento del stack
int m_bnb_pop(byte *out){
  addr_t stackTop = virtualMem[curAddr].stack + 1;
  addr_t blockStart = virtualMem[curAddr].start;
  addr_t blockEnd = virtualMem[curAddr].end;

  if (blockStart + blockEnd <= stackTop){
    return 1;
  }

  *out = m_read(stackTop);  // lee el valor en el tope del stack
  virtualMem[curAddr].stack++;  // actualiza la posición del stack
  return 0;
}


// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out){
  addr_t start = virtualMem[curAddr].start;
  addr_t end = virtualMem[curAddr].end;
  
  // si la dirección pertenece al bloque actual
  if (addr >= start && addr < end){
    *out = m_read(addr);
    return 0;
  }
  return 1;
}


// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val){
  addr_t addrStart = virtualMem[curAddr].start;
  addr_t curSize = virtualMem[curAddr].end;

  // si la dirección pertenece al bloque actual
  if (addr >= addrStart && addr < addrStart + curSize){
    m_write(addr, val);
    return 0;
  }
  return 1;
}


// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process){
  curPID = process.pid; // actualiza el pid del proceso actual
  curAddr = procsAddr[process.pid]; // actualiza la dirección actual
}


// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process){
  addr_t addr = procsAddr[process.pid];  // obtiene la dirección del proceso
  m_unset_owner(virtualMem[addr].start, virtualMem[addr].end);
  virtualMem[addr].onUse = 0;
  virtualMem[addr].owner = NO_OWNER;
  virtualMem[addr].size = 0;
  virtualMem[addr].heap = virtualMem[addr].start;
  virtualMem[addr].stack = virtualMem[addr].end;
}
