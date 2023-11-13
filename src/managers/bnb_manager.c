#include "bnb_manager.h"
#include <string.h>

#include "stdio.h"

int bounds;
ptr_t *spaces;
int currentContext;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  bounds = 1024;
  currentContext = 0;

  spaces = (ptr_t *)malloc(m_size()/bounds * sizeof(ptr_t));
  
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    spaces[i].ocupado = 0;
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
      // printf("No se encontró un bloque de memoria disponible del tamaño deseado.\n");
      if(spaces[currentContext].topHeap + size < spaces[currentContext].topStack)
      {
        insert(&spaces[currentContext].freeList, spaces[currentContext].addr + spaces[currentContext].topHeap,  size);
        spaces[currentContext].topHeap += size;
        bloque = search(&spaces[currentContext].freeList, size);
      }
      else
        return 1;
  }


  out->size = size;
  out->addr = bloque->value - spaces[currentContext].addr;//direccion virtual

  
  // Eliminar bloque de memoria disponible
  delete(&spaces[currentContext].freeList, bloque);
  insert(&spaces[currentContext].freeList, bloque->value + size ,bloque->size - size);


  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

  insert(&spaces[currentContext].freeList, ptr.addr, 1);

  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {

  if(spaces[currentContext].topHeap == spaces[currentContext].topStack-1)
    return 1;

  m_write( spaces[currentContext].addr + spaces[currentContext].topStack, val);
  out->addr = m_read(spaces[currentContext].addr + spaces[currentContext].topStack);

  spaces[currentContext].topStack--;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  spaces[currentContext].topStack++;
  *out = m_read(spaces[currentContext].addr + spaces[currentContext].topStack);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  *out = m_read(spaces[currentContext].addr + spaces[currentContext].process.program->size + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  m_write(spaces[currentContext].addr + spaces[currentContext].process.program->size + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

  int foundIt = 0;
  int space = -1;
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
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
    spaces[space].topHeap = process.program->size +1;
    currentContext = space;
    m_set_owner(spaces[space].addr, spaces[space].addr + bounds-1);
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  for (size_t i = 0; i < m_size()/bounds; i++)
  {
    if(spaces[i].ocupado && spaces[i].process.pid == process.pid){
      m_unset_owner(spaces[currentContext].addr, spaces[currentContext].addr + bounds-1);
      spaces[i].ocupado = 0;
      return;
    }
  }
  
}
