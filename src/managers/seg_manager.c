#include "seg_manager.h"

#include "stdio.h"
#include <string.h>

int size;
ptr_t *procesos;
int currentIndex;
ptr_t *memory;

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) {
  
  size = 128;
  currentIndex = 0;
  
  memory = (ptr_t *)malloc(sizeof(ptr_t));
  memset(&memory[0].freeList, 0, sizeof(FreeList));
  insert(&memory[0].freeList, 0, m_size());
  
  procesos = (ptr_t *)malloc((m_size()/size)/3 * sizeof(ptr_t));
  
  for (size_t i = 0; i < (m_size()/size)/3; i++)
  {
    procesos[i].ocupado = 0;
    procesos[i].process.pid = -1;
    procesos[i].addr = -1;
    procesos[i].size = -1;
    procesos[i].topStack = -1;
    memset(&procesos[i].Stack, 0, sizeof(FreeList));
  }

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {

  struct Node* bloque = search(&memory[0].freeList, size);
  out->size = size;
  out->addr = bloque->value;//direccion fisica

  // Eliminar bloque de memoria disponible
  delete(&memory[0].freeList, bloque);
  insert(&memory[0].freeList, bloque->value + size ,bloque->size - size);

  m_set_owner(bloque->value, bloque->value + size);

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {

  insert(&memory[0].freeList, ptr.addr, 1);

  return 0;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {

  insert(&procesos[currentIndex].Stack, procesos[currentIndex].topStack, 1);
  m_set_owner(procesos[currentIndex].topStack, procesos[currentIndex].topStack +1);
  m_write( procesos[currentIndex].topStack, val);
  out->addr = procesos[currentIndex].topStack;
  
  procesos[currentIndex].topStack++;

  return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {
  procesos[currentIndex].topStack--;
 
  *out = m_read(searchL(&procesos[currentIndex].Stack)->value);
  deleteLast(&procesos[currentIndex].Stack); 
  return 0;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {

  int foundIt = 0;
  int space = -1;

  for (size_t i = 0; i < (m_size()/size)/3; i++)
  {
    if (procesos[i].process.pid == process.pid)
    {
      currentIndex = i;
      return;
    }
  
    if(!foundIt && !procesos[i].ocupado){
      foundIt = 1;
      space = i;
    }
  }

  if(space != -1)
  {
    procesos[space].ocupado = 1;
    procesos[space].process = process;
    currentIndex = space;

    //espacio de codigo
    struct Node* bloque = search(&memory[0].freeList, process.program->size);
    
    delete(&memory[0].freeList, bloque);
    insert(&memory[0].freeList, bloque->value + process.program->size, bloque->size - process.program->size);
    
    int sizeStack = 100;
    struct Node* bloque2 = search(&memory[0].freeList, sizeStack);
    procesos[currentIndex].topStack = bloque2->value;//direccion fisica

    delete(&memory[0].freeList, bloque2);
    insert(&memory[0].freeList, bloque2->value + sizeStack, bloque2->size - sizeStack);

  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  for (size_t i = 0; i < (m_size()/size)/3; i++)
  {
    if(procesos[i].ocupado && procesos[i].process.pid == process.pid){
      procesos[i].ocupado = 0;
      return;
    }
  }
}
