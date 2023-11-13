#include "bnb_manager.h"

#include "stdio.h"
#include "structs.h"

//excepciones en los void
int proceso;
Piece_t *Memory;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  Memory = (Piece_t *) malloc(m_size()/512*sizeof(Piece_t));
  int base = 0;
  for (size_t i = 0; i < m_size()/512; i++)
  {
    Piece_t p;
    p.base = base;
    base += 512;
    for (size_t j = 0; j < 512; j++)
    {
      p.bytes[j] = 0;
    }
    p.process = -1;
    Memory[i] = p;
  }
  
  proceso = -1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  // hay espacio para reservar
  int is = 0;
  // direccion del espacio a reservar
  int dir = -1;
  for (size_t i = 0; i < 512; i++)
  {
    if (Memory[proceso].bytes[i] == 0)
    {
      is = 1;
      dir = i;
      for (size_t j = 0; j < size; j++)
      {
        if (Memory[proceso].bytes[i+j] != 0)
        {
          is = 0;
          dir = -1;
          break;
        }
      }

    }
    if (is || Memory[proceso].bytes[i] == 3)
    // llego al stack
    {
      break;
    }
  }
  if (is)
  {
    for (size_t i = dir; i < dir + size; i++)
    {
      Memory[proceso].bytes[i] = 2;
    }
  }
  else 
  {
    printf("no cabe mi pana");
    return 1;
  }
  out->addr = Memory[proceso].base + dir; 
  out->size = size;

  return 0;
  
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  for (size_t i = ptr.addr % 512; i < ptr.size; i++) 
  {
    if (Memory[proceso].bytes[i] != 2)
    {
      return 1;
    }
    Memory[proceso].bytes[i] = 0;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  // esta lleno el stack
  int full = 0;

  // busca el primer 3 (stack) y añade uno en la posicion anterior si esta 
  //no es un 2 o 1 (heap o codigo respectivamente)
  for (size_t i = 0; i < 512; i++)
  {
    if (Memory[proceso].bytes[i] == 3)
    {
      if (Memory[proceso].bytes[i-1] == 0)
      {
        Memory[proceso].bytes[i-1] = 3;
        out->addr = Memory[proceso].base +i-1;
        out->size = 1;
        m_write(Memory[proceso].base +i-1,val); 
        return 0;
      }
      full = 1;
    }
  }
  if (!full)
  {
    if (Memory[proceso].bytes[511] == 0)
    {
      Memory[proceso].bytes[511] = 3;
      out->addr = Memory[proceso].base +511;
      out->size = 1;
      m_write(Memory[proceso].base +511, val); 
      return 0;
    }
  }
  
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  // busca el primer 3 (tope del stack) y devuelve el valor corresp
  for (size_t i = 0; i < 512; i++)
  {
    if (Memory[proceso].bytes[i] == 3)
    {
      Memory[proceso].bytes[i] = 0;
      *out = m_read(Memory[proceso].base + i); 
      return 0;
    }
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if (addr-(addr % 512) != Memory[proceso].base)
  {
    return 1;
  }

  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  if (addr-(addr%512) != Memory[proceso].base)
  {
    return 1;
  }
  if (Memory[proceso].bytes[addr%512] == 2)
  {
    
    m_write(addr,val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  proceso = process.pid;
  // el proceso ya tiene un fragmento de memoria asignado
  int is = 0;
  // hay un fragmento de memoria vacio para un nvo proceso
  int st = 0;
  // direccion del primer fragmento de memoria vacio
  int first = -1;
  for (size_t i = 0; i < m_size()/512; i++)
  {
    if (Memory[i].process == -1 && !st)
    {
      first = i;
      st = 1;
    }
    if (Memory[i].process == proceso)
    {
      is = 1;
      break;
    }
  }

  if (!st && !is)
  // si no esta y no hay donde ponerlo 
  {
    printf("esto no cabe");
    return;
  }

  if (!is)
  // si no esta se ubica
  {
    Memory[first].process = proceso;
    m_set_owner(Memory[first].base, Memory[first].base + 511);
    for (size_t i = 0; i < process.program->size; i++)
    {
      Memory[proceso].bytes[i] = 1;
    }
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  // libera el fragemnto de memoria y este puede ser utilizado por otro proceso
  for (size_t i = 0; i < m_size()/512; i++)
  {
    if (Memory[i].process == process.pid)
    {
      Memory[i].process == -1;
      m_unset_owner(Memory[i].base, Memory[i].base + 511);
      for (size_t j = 0; j < 512; j++)
      {
        Memory[i].bytes[j] = 0;
      }
      
    }
  }
}
