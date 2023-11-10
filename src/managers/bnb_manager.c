#include "bnb_manager.h"
#include "stdio.h"
#include "../bnb_process.h"

// definir variables globales
bnb_process_t *bnb_process;
int current_position = -1;
int slice = 512;
int ammount = -1;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  // inicializo el array de procesos
  ammount = m_size() / slice;
  bnb_process = (bnb_process_t *)malloc(ammount * sizeof(bnb_process_t));
  // por cada proceso voy estableciendo los valores que quiero guardar de cada uno.
  for (int i = 0; i < ammount; i++)
  {
    bnb_process[i].base = slice * i;
    bnb_process[i].stack_pointer = slice-1; //! puede que haya q restarle 1 o sumarle 1 a los bases
    bnb_process[i].pid = -1;
    bnb_process[i].bound = (slice *i) + slice;
    bnb_process[i].inst_pointer = 1;
    for (int j = 0; j < slice; j++)
    {
      bnb_process[i].heap[j] = 0;
    }
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  int start_address = bnb_process[current_position].code_size;
  int from = 0;
  // recorro el heap para ver si es posible reservar.
  for (int i = 0; i < bnb_process[current_position].stack_pointer; i++)
  {
    if (start_address + i < bnb_process[current_position].stack_pointer) // comprueba que el heap y el stack no se crucen.
    {
      if (bnb_process[current_position].heap[i] == 0) // espacio disponible en el heap
      {
          from = i;
          break;
      }
    }
    return 1;
  }
  // reservo en el heap el espacio solicitado.
  for (int k = from; k < from + size ; k++)
  {
    bnb_process[current_position].heap[k] = 1;
  }
  out->addr = bnb_process[current_position].code_size + from; // guarda la direccion correspondiente al proceso.
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  for (int i = ptr.addr; i < (int)(ptr.addr + ptr.size); i++)
  {
    bnb_process[current_position].heap[i] = 0;
  }
  return 0; 
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if ((bnb_process[current_position].stack_pointer - 1) >= slice)
  {
    // ! Ajustar el ip para poner bien el error.
    return 1;
  }
  m_write(bnb_process[current_position].stack_pointer + bnb_process[current_position].base, val);
  bnb_process[current_position].stack_pointer --;
  out->addr = bnb_process[current_position].stack_pointer; 
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (bnb_process[current_position].stack_pointer >= slice - 1)
  {
    return 1;
  }
  *out = m_read(bnb_process[current_position].base + bnb_process[current_position].stack_pointer + 1);
  bnb_process[current_position].stack_pointer++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  //? es probable que necesite hacer otro if antes para comprobar que la dirección sea válida.
  // compruebo que la direccion tenga algún valor .
  if (bnb_process[current_position].heap[addr - bnb_process[current_position].code_size] == 1)
  {
    *out = m_read(addr + bnb_process[current_position].base);
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  //? si la posicion del heap es válida pero está ocupada devuelvo error??
  if (bnb_process[current_position].heap[addr - bnb_process[current_position].code_size] == 1)
  {
    m_write(addr + bnb_process[current_position].base, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  // verificar si el proceso ya se encuentra.
  for (int i = 0; i < ammount; i++)
  {
    if (bnb_process[i].pid == process.pid)
    {
      current_position = i;
      return;
    }
  }
  // Si el proceso no está entonces lo agrego y actualizo todas las variables.
  for (int i = 0; i < ammount; i++)
  {
    if (bnb_process[i].pid == -1)
    {
      bnb_process[i].pid = process.pid;
      bnb_process[i].code_size = process.program->size;
      current_position = i;
      m_set_owner(bnb_process[current_position].base, bnb_process[current_position].base + slice - 1);
      break;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (int i = 0; i < ammount; i++)
  {
    if (bnb_process[i].pid == process.pid)
    {
      bnb_process[i].pid = -1;
      m_unset_owner(bnb_process[i].base, bnb_process[i].base + slice - 1);
      break;
    }
    //? Tendré que hacer free??
  }
}
