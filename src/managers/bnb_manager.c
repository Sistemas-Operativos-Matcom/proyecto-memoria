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
  for (size_t i = 0; i < ammount; i++)
  {
    bnb_process[i].base = slice * i;
    bnb_process[i].stack_pointer = slice; //! puede que haya q restarle 1 o sumarle 1 a los bases
    bnb_process[i].pid = -1;
    for (size_t j = 0; j < slice; j++)
    {
      bnb_process[i].heap[j] = 0;
    }
  }

  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  int start_address = bnb_process[current_position].code_size;
  int from = -1;
  int to = -1;
  int ready = 0;
  // recorro el heap para ver si es posible reservar.
  for (size_t i = 0; i < bnb_process[current_position].stack_pointer; i++)
  {
    if (start_address + i < bnb_process[current_position].stack_pointer) // comprueba que el heap y el stack no se crucen.
    {
      if (bnb_process[current_position].heap[i] == 0) // espacio disponible en el heap
      {
        if (!ready)
        {
          from = i;
          ready = 1;
        }
        if (i - from == size) // cumple con el tamaño pedido.
        {
          to = i;
          break;
        }
      }
    }
  }
  // reservo en el heap el espacio solicitado.
  for (size_t k = from; k < to + 1; k++)
  {
    bnb_process[current_position].heap[k] = 1;
  }
  out->addr = bnb_process[current_position].code_size + from; // guarda la direccion correspondiente al proceso.

  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  for (size_t i = ptr.addr; i < ptr.addr + ptr.size; i++)
  {
    bnb_process[current_position].heap[i] = 0;
  }

  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if ((bnb_process[current_position].stack_pointer - 1) <= slice)
  {
    return 1;
  }
  m_write(bnb_process[current_position].stack_pointer + bnb_process[current_position].base, val);
  out = 0; //! creo que con hacer un read puedo resolver la direccion.
  return 0;
  printf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (bnb_process[current_position].stack_pointer <= slice - 1)
  {
    //! devolver error.
  }

  *out = m_read(bnb_process[current_position].base + bnb_process[current_position].stack_pointer + 1);
  bnb_process[current_position].stack_pointer++;
  fprintf(stderr, "Not Implemented\n");
  exit(1);
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
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  //? si la posicion del heap es válida pero está ocupada devuelvo error??
  if (bnb_process[current_position].heap[addr - bnb_process[current_position].code_size] == 0)
  {
    m_write(addr + bnb_process[current_position].base, val);
    return 0;
  }
  return 1;
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  // verificar si el proceso ya se encuentra.
  for (size_t i = 0; i < ammount; i++)
  {
    if (bnb_process[i].pid == process.pid)
    {
      current_position = i;
      break;
    }
  }
  // Si el proceso no está entonces lo agrego y actualizo todas las variables.
  for (size_t i = 0; i < ammount; i++)
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
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (size_t i = 0; i < ammount; i++)
  {
    if (bnb_process[i].pid == process.pid)
    {
      bnb_process[i].pid = -1;
      m_unset_owner(bnb_process[i].base, bnb_process[i].base + slice - 1);
      break;
    }
    //? Tendré que hacer free??
  }

  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
