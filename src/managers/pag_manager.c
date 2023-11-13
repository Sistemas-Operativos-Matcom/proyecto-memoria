#include "pag_manager.h"

#include "stdio.h"
#include <string.h>

ptr_t *spacesP;
ptr_t *pages;
int sizePage;
int maxProc;
int currentP;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  sizePage = 64;
  maxProc = 10;
  currentP = 0;

  spacesP = (ptr_t *)malloc(maxProc * sizeof(ptr_t));
  pages = (ptr_t *)malloc(m_size()/sizePage * sizeof(ptr_t));
  
  for (int i = 0; i < maxProc; i++)
  {
    spacesP[i].ocupado = 0;
  }

  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    pages[i].addr = i*sizePage;
    pages[i].ocupado = 0;
    pages[i].size = 0;
    pages[i].process.pid = -1;
  }

  for (int i = 0; i < maxProc; i++)
  {
    spacesP[i].process.pid = -1;

    spacesP[i].topStack = 0;
    spacesP[i].topHeap = 0;
    memset(&spacesP[i].freeList, 0, sizeof(FreeList));
    memset(&spacesP[i].Stack, 0, sizeof(FreeList));
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {

  int free = -1;
  int cont = 0;

  int countPages = 0;
  if(size%sizePage == 0)
    countPages = size/sizePage;
  else
    countPages = size/sizePage +1;

  //Revisa los page frame
  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    if(pages[i].ocupado == 0)
    {
      free = i;
      cont++;
    }
    else
    {
      free = -1;
      cont = 0;
    }
    
    if(cont  == countPages)
    {
      free = free - cont +1;
      break;
    }
  }

  if(free != -1)
  {
    m_set_owner(pages[free].addr, pages[free].addr + sizePage*countPages);

    for (int i = 0; i < countPages; i++)
    {
      pages[i].process.pid = currentP;
      pages[free + i].ocupado = 1;
      pages[free +i].size = sizePage;
      insert(&spacesP[currentP].freeList, free+i, 0);
    }

    out->size = size;
    out->addr = pages[free].addr;//direccion fisica
  }
  else
    return 1;
  
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  struct Node* bloque = search(&spacesP[currentP].freeList, ptr.addr);
  delete(&spacesP[currentP].freeList, bloque);
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  
  int free = -1;
  //Revisa los page frame
  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    
    if(pages[i].process.pid == currentP || pages[i].process.pid == -1)
    {
      if((int)pages[i].size < sizePage)
      {
        pages[i].process.pid = currentP;
        pages[i].ocupado = 1;
        pages[i].size++;
        free = i;
        break;
      }
    }
  }

  if(free != -1)
  {
    m_set_owner(pages[free].addr, pages[free].addr + sizePage);
    insert(&spacesP[currentP].Stack, pages[free].addr + pages[free].size, 1);
  }
  else
    return 1;
  
  m_write( pages[free].addr + pages[free].size, val);
  // printf("----%x\n", (pages[free].addr + pages[free].size)/ sizePage);
  out->addr = m_read(pages[free].addr + pages[free].size);

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  // printf("----%x\n", searchL(&spacesP[currentP].Stack)->value / sizePage);

  //Para saber de donde popear lee el ultimo elemento de mi stack, toma su direccion,
  // ahora necesita saber a q pagina pertenece esa direccion, por eso aprovecha q la division es
  // parte entera. Una vez tiene la pagina coge la referencia de esa pagina y le suma el cachito de pagina
  // que puede avanzar
  *out = m_read(pages[searchL(&spacesP[currentP].Stack)->value / sizePage].addr + searchL(&spacesP[currentP].Stack)->value % sizePage);
  pages[searchL(&spacesP[currentP].Stack)->value / sizePage].size--;

  if(pages[searchL(&spacesP[currentP].Stack)->value / sizePage].size == 0)
    pages[searchL(&spacesP[currentP].Stack)->value / sizePage].ocupado = 0;
  // FILE* hola;
  // hola = fopen("hola.txt", "a");
  // fprintf(hola, "----%d\n", 55);
  // fclose(hola);
  deleteLast(&spacesP[currentP].Stack); 

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {

  int foundIt = 0;
  int space = -1;

  for (int i = 0; i < maxProc; i++)
  {
    if (spacesP[i].process.pid == process.pid)
    {
      currentP = i;
      return;
    }
  
    if(!foundIt && !spacesP[i].ocupado){
      foundIt = 1;
      space = i;
    }
  }

  if(space != -1)
  {
    spacesP[space].ocupado = 1;
    spacesP[space].process = process;
    currentP = space;

    //Por cada pagina q necesite para guardar el codigo
    for (size_t i = 0; i < process.program->size/sizePage; i++)
    {
      int free = -1;

      //Revisa los page frame
      for (size_t i = 0; i < m_size()/sizePage; i++)
      {
        if(pages[i].ocupado == 0)
        {
          pages[i].ocupado = 1;
          pages[i].size = sizePage;
          free = i;
          break;
        }
      }

      if(free != -1)
      {
        m_set_owner(pages[free].addr, pages[free].addr + sizePage);
        insert(&spacesP[currentP].freeList, free, 0);
      }

    }
  }

}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {

  for (int i = 0; i < maxProc; i++)
  {
    if(spacesP[i].ocupado && spacesP[i].process.pid == process.pid){
      spacesP[i].ocupado = 0;
      break;
    }
  }

  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    if(pages[i].ocupado && pages[i].process.pid == process.pid){
      pages[i].ocupado = 0;
      pages[i].size = 0;
      m_unset_owner(pages[i].addr, pages[i].addr + sizePage);
      pages[i].process.pid = -1;
    }
  }
}
