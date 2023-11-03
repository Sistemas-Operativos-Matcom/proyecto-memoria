#include "bnb_manager.h"
#include <string.h>

#include "stdio.h"

int bounds;
ptr_t *spaces;
int currentContext;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  // // memset(&freeList, 0, sizeof(FreeList));
  bounds = 1024;
  currentContext = 0;

  spaces = (ptr_t *)malloc(m_size()/bounds * sizeof(ptr_t));

  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    spaces[i].process.pid = -1;
    spaces[i].addr = bounds*i;
    spaces[i].size = bounds;
    spaces[i].topStack = bounds-1;
    memset(&spaces[i].freeList, 0, sizeof(FreeList));
  }
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  
  struct Node* bloque = search(&spaces[currentContext].freeList, size);
  if (bloque != NULL) {
      // printf("Bloque de memoria disponible de tamaño %d encontrado.\n", bloque->size);
  } else {
      printf("No se encontró un bloque de memoria disponible del tamaño deseado.\n");
      return 1;
  }
  // printf("\n");

  out->size = size;
  out->addr = bloque->value;

  // printf("puntero reservaI -- %d\n", out->addr);

  
  // Eliminar bloque de memoria disponible
  delete(&spaces[currentContext].freeList, bloque);
  insert(&spaces[currentContext].freeList, bloque->value + size ,bloque->size - size);


  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

  insert(&spaces[currentContext].freeList, ptr.addr, ptr.size);

  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {

  out = spaces[currentContext].topStack;
  m_write( spaces[currentContext].addr + spaces[currentContext].topStack, val);
  spaces[currentContext].topStack--;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  spaces[currentContext].topStack = spaces[currentContext].topStack + spaces[currentContext].addr + 1;
  // printf("hola4 -- %d\n",  spaces[currentContext].topStack);
  *out = spaces[currentContext].topStack;
  printf("hola4 -- %d\n", *out);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  // printf("%d\n---------------",  addr);
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

  int foundIt = 0;
  int space = -1;
  // // printf("%d\n---------------", ((process_t)(&spaces[0].addr)).pid);
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    // printf("hola %d\n", spaces[i].process.pid);
    if (spaces[i].process.pid == process.pid)
    {
      currentContext = i;
      return;
    }
  
    if(!foundIt && !spaces[i].ocupado){
      foundIt = 1;
      space = i;
    }
  }

  if(space != -1)
  {

    spaces[space].ocupado = 1;
    spaces[space].process = process;
    currentContext = space;
    m_set_owner(spaces[currentContext].addr, spaces[currentContext].addr + bounds-1);
    insert(&spaces[space].freeList, spaces[currentContext].addr + process.program->size, (bounds - process.program->size)/2 );
    // printf("codigo -- %d\n", process.program->size);
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
