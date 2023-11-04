#include "bnb_manager.h"

#include "stdio.h"
#include "structs.h"

//validar direcciones de memoria
int proceso;
List_t *Memory;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  Memory = (List_t *) malloc(sizeof(List_t)*m_size()/1024);
  proceso = -1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  int is = 0;
  int dir = -1;
  for (size_t i = 0; i < 1024; i++)
  {
    if (Memory->pieces[proceso].bytes[i] == 0)
    {
      is = 1;
      for (size_t j = 0; j < size; j++)
      {
        if (Memory->pieces[proceso].bytes[i+j] != 0)
        {
          is = 0;
          break;
        }
      }
      dir = i;
    }
    if (is || Memory->pieces[proceso].bytes[i] == 3)
    {
      break;
    }
  }
  if (is)
  {
    for (size_t i = dir; i < size; i++)
    {
      Memory->pieces[proceso].bytes[i] = 2;
    }
  }
  else 
  {
    printf("no cabe mi pana");
    return 1;
  }
  out->addr = Memory->pieces[proceso].base + dir; 
  out->size = size;

  return 0;
  
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  for (size_t i = ptr.addr % 1024; i < ptr.size; i++) 
  {
    if (Memory->pieces[proceso].bytes[i] != 2)
    {
      return 1;
    }
    Memory->pieces[proceso].bytes[i] = 0;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  int full = 0;
  for (size_t i = 0; i < 1024; i++)
  {
    if (Memory->pieces[proceso].bytes[i] == 3)
    {
      if (Memory->pieces[proceso].bytes[i-1] == 0)
      {
        Memory->pieces[proceso].bytes[i-1] = 3;
        out->addr = Memory->pieces[proceso].base +i-1;
        out->size = 1;
        m_write(Memory->pieces[proceso].base +i-1,val); 
        return 0;
      }
      full = 1;
    }
  }
  if (!full)
  {
    if (Memory->pieces[proceso].bytes[1023] == 0)
    {
      out->addr = Memory->pieces[proceso].base +1023;
      out->size = 1;
      m_write(Memory->pieces[proceso].base +1023, val); 
      return 0;
    }
  }
  
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  for (size_t i = 0; i < 1024; i++)
  {
    if (Memory->pieces[proceso].bytes[i] == 3)
    {
      Memory->pieces[proceso].bytes[i] = 0;
      out = m_read(Memory->pieces[proceso].base + i); 
      return 0;
    }
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if (addr/1024 != Memory->pieces[proceso].base)
  {
    return 1;
  }
  out = m_read(addr); // asi se facil?
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  if (addr/1024 != Memory->pieces[proceso].base)
  {
    return 1;
  }
  m_write(addr,val); // asi de facil?
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  proceso = process.pid;
  int is = 0;
  int st = 0;
  int first = -1;
  for (size_t i = 0; i < m_size()/1024; i++)
  {
    if (Memory->pieces[i].process == -1 && !st)
    {
      first = i;
      st = 1;
    }
    if (Memory->pieces[i].process == proceso)
    {
      is = 1;
      break;
    }
  }

  if (st)
  {
    printf("esto no cabe");
    return;
  }

  if (!is)
  {
    Memory->pieces[first].process = proceso;
    for (size_t i = 0; i < process.program->size; i++)
    {
      Memory->pieces[proceso].bytes[i] = 1;
    }
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  for (size_t i = 0; i < m_size()/1024; i++)
  {
    if (Memory->pieces[i].process == process.pid)
    {
      Memory->pieces[i].process == -1;
      for (size_t j = 0; j < 1024; j++)
      {
        Memory->pieces[i].bytes[j] = 0;
      }
      
    }
  }
}
