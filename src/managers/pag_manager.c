#include "pag_manager.h"

#include "stdio.h"

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
  
  for (size_t i = 0; i < maxProc; i++)
  {
    spacesP[i].ocupado = 0;
  }

  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    pages[i].addr = i*sizePage;
    pages[i].ocupado = 0;
    pages[i].size = 0;
  }

  for (size_t i = 0; i < maxProc; i++)
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

    for (size_t i = 0; i < countPages; i++)
    {
      pages[free + i].ocupado = 1;
      insert(&spacesP[currentP].freeList, free+i, 0);
    }

    out->size = size;
    out->addr = pages[free].addr;
  }
  else
    return 1;
    
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  
  int free = -1;
  //Revisa los page frame
  for (size_t i = 0; i < m_size()/sizePage; i++)
  {
    if(pages[i].ocupado == 0)
    {
      pages[i].ocupado = 1;
      free = i;
      break;
    }
  }

  if(free != -1)
  {
    m_set_owner(pages[free].addr, pages[free].addr + 1);
    insert(&spacesP[currentP].Stack, free, 1);
  }
  else
    return 1;
  
  m_write( pages[free].addr, val);
  out = m_read(pages[free].addr);

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
    
  *out = m_read(pages[searchL(&spacesP[currentP].Stack)->value].addr);
  pages[searchL(&spacesP[currentP].Stack)->value].ocupado = 0;
  // FILE* hola;
  // hola = fopen("hola.txt", "a");
  // fprintf(hola, "----%d\n", 55);
  // fclose(hola);
  deleteLast(&spacesP[currentP].Stack); 
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

  for (size_t i = 0; i < maxProc; i++)
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
          free = i;
          break;
        }
      }

      if(free != -1)
      {
        m_set_owner(pages[free].addr, pages[free].addr + sizePage);
        insert(&spacesP[currentP].freeList, free, 0);
      }
      else
        return 1;
    }
  }

}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {

  for (size_t i = 0; i < maxProc; i++)
  {
    if(spacesP[i].ocupado && spacesP[i].process.pid == process.pid){
      spacesP[i].ocupado = 0;
      return;
    }
  }
}
