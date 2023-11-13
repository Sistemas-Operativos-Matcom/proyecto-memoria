#include "bnb_manager.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct memory {
    int active;
    int use;
    int pid;
    addr_t start;
    addr_t end;
    addr_t stack;
    addr_t heap;
    size_t size;
}process_memory_t;

int maxProcessAmount = 32;
int pid;
addr_t currentAddr;
addr_t *processAddr;
process_memory_t *processMem;
int memorySize;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  free(processAddr);
  free(processMem);


  processAddr = (size_t *)malloc(maxProcessAmount * sizeof(size_t));
  processMem = (process_memory_t *)malloc(maxProcessAmount * sizeof(process_memory_t));
  currentAddr = 0;
  memorySize = m_size()/maxProcessAmount;
  
  int start = 0;
  for(int i = 0; i < maxProcessAmount; i++){
    processMem[i].start = start+1;
    processMem[i].heap = start+1;
    processMem[i].end = start+(memorySize-1);
    processMem[i].stack = start+(memorySize-1);
    processMem[i].size = 0;
    processMem[i].active = 0;
    processMem[i].use = NO_ONWER;

    start+= memorySize;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  for(int i = 0; i < maxProcessAmount; i++){
    if(!processMem[i].active){
        m_set_owner(((i*memorySize)+1), ((i*memorySize)+(memorySize-1)));
        processMem[i].active = 1;
        processMem[i].use = pid;
        processMem[i].size = size;
        processAddr[pid] = i;
        currentAddr = i;
        out->addr = (i*memorySize) + 1;
        out->size = size;
        return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  if(ptr.addr >= processMem[currentAddr].start && ptr.size < processMem[currentAddr].end){
    m_unset_owner(ptr.addr, ptr.addr + ptr.size -1);
    processMem[currentAddr].size -= ptr.size;
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if(processMem[currentAddr].stack-1 <= processMem[currentAddr].heap)return 1;
  m_write(processMem[currentAddr].stack, val);
  processMem[currentAddr].stack--;
  out->addr = processMem[currentAddr].stack;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if(processMem[currentAddr].stack + 1 >= processMem[currentAddr].end + processMem[currentAddr].start) return 1;
  
  *out = m_read(processMem[currentAddr].stack + 1);
  processMem[currentAddr].stack++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if(addr >= processMem[currentAddr].start && addr < processMem[currentAddr].end)
    {
        *out = m_read(addr);
        return 0;
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if(addr >= processMem[currentAddr].start && addr <processMem[currentAddr].start + processMem[currentAddr].size)
  {
    m_write(addr, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  pid = process.pid;
  currentAddr = processAddr[pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  m_unset_owner(processMem[currentAddr].start, processMem[currentAddr].end);
  processMem[processAddr[process.pid]].stack = processMem[processAddr[process.pid]].end;
  processMem[processAddr[process.pid]].heap = processMem[processAddr[process.pid]].start;
  processMem[processAddr[process.pid]].active = 0;
  processMem[processAddr[process.pid]].use = NO_ONWER;
  processMem[processAddr[process.pid]].size = 0;
}




