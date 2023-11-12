#include "pag_manager.h"

#include "stdio.h"
#include "structs.h"

int proceso;
Page_t *Memory;
Proc_t *Procs;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  //esto hay q arreglarlo
  Memory = (Page_t *) malloc(m_size()/32*sizeof(Page_t));
  Procs = (Proc_t *) malloc(1000*sizeof(Proc_t));
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  if (Procs[proceso].lpheap == -1)
  //ehh y si es mas grande que 32???
  {
    //Buscar una pag vacia y asig space
    Procs[proceso].lpheap ++;
    // set owner
    out->addr = Procs[proceso].code[0].base;
    out->size = size;
    return 0;
  }

  for (size_t i = 0; i < Procs[proceso].lpheap +1; i++)
  {
    //ubic en las pags
  }
  


}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if (Procs[proceso].lpstack == -1)
  {
    for (size_t i = 0; i < m_size()/32; i++)
    {
      if (!Memory[i].inUse)
      {
        Memory[i].inUse = 1;
        Procs[proceso].lpstack++;
        Procs[proceso].stack[Procs[proceso].lpstack] = Memory[i];
        break;
      }
    }
  }
  for (size_t i = 0; i < 32; i++)
  {
    if (Procs[proceso].stack[Procs[proceso].lpstack].bytes[i] == 0)
    {
      //escibe y bla bla
      return 0;
    }
  }
  for (size_t i = 0; i < m_size()/32; i++)
  {
    if (!Memory[i].inUse)
    {
      Memory[i].inUse = 1;
      Procs[proceso].lpstack++;
      Procs[proceso].stack[Procs[proceso].lpstack] = Memory[i];
      Procs[proceso].stack[Procs[proceso].lpstack].bytes[0] = 1;
      // escribe y bla bla
      return 0;
    }
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  for (size_t i = 31; i <= 0; i++)
  {
    if (Procs[proceso].stack[Procs[proceso].lpstack].bytes[i])
    {
      Procs[proceso].stack[Procs[proceso].lpstack].bytes[i] = 0;
      if (i == 0)
      {
        Procs[proceso].lpstack --;
      }
      // leer y to eso
      return 0;
    }
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  *out = m_read(addr); // asi se facil?
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  m_write(addr,val); // asi se facil?
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  int is = 0;
  for (size_t i = 0; i < 1000; i++)
  {
    if (process.pid == Procs[i].pid)
    {
      proceso = i;
      is = 1;
      break;
    }
  }
  if (!is)
  {
    for (size_t i = 0; i < 1000; i++)
    {
      if(Procs[i].pid == -1)
      {
        proceso = i;
        break;
      }
    }
    int code = process.program->size;
    for (size_t i = 0; i < m_size()/32; i++)
    {
      if (!Memory[i].inUse)
      {
        Memory[i].inUse = 1;
        Procs[proceso].code[Procs[proceso].lpcode] = Memory[i];
        Procs[proceso].lpcode ++;
        code -= 32;
      }
      if (code<=0)
      {
        break;
      }
    }
    
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
