#include "bnb_manager.h"
#include <string.h>

#include "stdio.h"

int base;
int bounds;
ptr_t *spaces;
int currentContext;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  // memset(&freeList, 0, sizeof(FreeList));
  base = 0;
  bounds = 1024;
  currentContext = 0;

  spaces = (ptr_t *)malloc(m_size()/bounds * sizeof(ptr_t));
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  
  // out = spaces[currentContext].addr;
  // spaces[currentContext].addr += size;

  struct Node* bloque = search(&spaces[currentContext].freeList, size);
  if (bloque != NULL) {
      printf("Bloque de memoria disponible de tamaño %d encontrado.\n", bloque->size);
  } else {
      printf("No se encontró un bloque de memoria disponible del tamaño deseado.\n");
      return 1;
  }
  printf("\n");

  // out->process = bloque->value;  //REVISAR ESTO!!!!!!!!!!!
  out->size = bloque->size;
  out->ocupado = 1;
  out->addr = &bloque->value;

  // Eliminar bloque de memoria disponible
  delete(&spaces[currentContext].freeList, bloque);


  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

  insert(&spaces[currentContext].freeList, ptr.size);

  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  // push(spaces[currentContext].stack, val);

  out = --spaces[currentContext].topStack;
  m_write(spaces[currentContext].topStack, val);
  
  // out = *(spaces[currentContext].stack.array[ spaces[currentContext].stack.front ])
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  // pop(spaces[currentContext].stack, &out);
  out = &spaces[currentContext].topStack;
  spaces[currentContext].topStack++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  out = m_read(addr - base);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  m_write(addr - base, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

  int foundIt = 0;
  int space = -1;
  // printf("%d\n---------------", ((process_t)(&spaces[0].addr)).pid);
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    if (spaces[i].process.pid == process.pid)
    {
      currentContext = i;
      foundIt = 1;

      return;
    }
  
    if(!spaces[i].ocupado){
      space = i;
    }
  }

  if(!foundIt && space != -1)
  {
    spaces[space].ocupado = 1;
    spaces[space].process = process;
    memset(&spaces[space].freeList, 0, sizeof(FreeList));
    // spaces[i].size;  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    spaces[space].topStack = bounds;
    currentContext = space;
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    if(spaces[i].ocupado && spaces[i].process.pid == process.pid){
      spaces[i].ocupado = 0;
      return;
    }
  }
  
}
