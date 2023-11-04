#include "bnb_manager.h"
#include "stdio.h"
#include "structs.h"

#define Max_Process_Count 10
#define Off 0
#define On 1

// Declarando variables universales:
static int initialized = Off;       //Determina si las estructuras están inicializadas
static Fragment *Fragments;         //Lista de fragmentos de memoria
static size_t frag_size = 0;        //Tamaño de los fragmentos de memoria
static int current_fragment = -1;   //Fragmento actual en el que estamos operando
static int last_assigned = -1;      //Último fragmento de memoria asignado

//----------------------------------------------------------IMPLEMENTACIÓN------------------------------------------------------------------------------

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  // Si ya la memoria se inicializó previamente, limpiarla
  if (initialized)
  {
    free(Fragments);
  }
  // Asignando valores iniciales
  frag_size = m_size() / Max_Process_Count;                             // El tamaño del fragmento de memoria será el mismo para todos los fragmentos
  Fragments = (Fragment *)malloc(Max_Process_Count * sizeof(Fragment)); // Asignando espacio para el heap
  last_assigned = -1;                                                   // Aún no hay fragmentos de memoria asignados
  current_fragment = -1;
  initialized = On;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  // Buscando el espacio adecuado
  List *list = &Fragments[current_fragment].heap;
  size_t addr = -1;
  // Reservando memoria
  int returned = Get_Mem(list, size, &addr);
  // Guardar la direccion obtenida
  out->addr = (size_t)addr;
  // Retornando valor obtenido (Ok o Error)
  return returned;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  // Buscando el espacio adecuado
  List *list = &Fragments[current_fragment].heap;
  // Liberando memoria
  return Free_Mem(list, ptr.size, (int)ptr.addr);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  // Verificando que el stack pointer esté dentro de los límites permitidos
  if (Fragments[current_fragment].heap_pointer >= Fragments[current_fragment].stack_pointer - 1)
  {
    return Error;
  }
  // Obteniendo la dirección de memoria física del valor
  size_t val_stackpointer = Fragments[current_fragment].stack_pointer;
  size_t process_stackpointer = Fragments[current_fragment].base + (--val_stackpointer);
  Fragments[current_fragment].stack_pointer--;
  // Guardando el valor en stack
  m_write(process_stackpointer, val);
  // Actualizando punteros
  out->addr = val_stackpointer;
  out->size = 1;
  return Ok;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  size_t val_stackpointer = Fragments[current_fragment].stack_pointer;
  // Si el stackpointer se pasa del tamaño máximo, devuelve error
  if ((int)val_stackpointer + 1 > (int)frag_size)
  {
    return Error;
  }
  // Quitar el elemento correspondiente de la stack
  int addr = (int)val_stackpointer + Fragments[current_fragment].base;
  *out = m_read(addr);
  // Actualizando puntero de stack
  Fragments[current_fragment].stack_pointer++;
  return Ok;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  // Si la direccion no pertenece al fragmento de memoria correspondiente, devuelve error
  if (addr >= (addr_t)Fragments[current_fragment].bound)
  {
    return Error;
  }
  // Obteneniendo la dirección de memoria física correspondiente
  int p_addr = Fragments[current_fragment].base + (int)addr;
  // Asignando valor
  *out = m_read(p_addr);
  return Ok;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  // Si la direccion no pertenece al fragmento de memoria correspondiente, devuelve error
  if (addr >= (addr_t)Fragments[current_fragment].bound)
  {
    return Error;
  }
  // Obteneniendo la dirección de memoria física correspondiente
  int p_addr = Fragments[current_fragment].base + (int)addr;
  // Asignando valor en la direccion correcta
  m_write(p_addr, val);
  return Ok;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  int last_empty_fragment = -1;
  // Buscar el proceso en la lista de fragmentos de memoria
  for (int i = 0; i <= last_assigned; i++)
  {
    // Si se encuentra se retorna
    if (Fragments[i].process_pid == process.pid)
    {
      current_fragment = i;
      return;
    }
    // Si hay un espacio vacío, se guarda
    else if (Fragments[i].process_pid == -1)
    {
      last_empty_fragment = i;
    }
  }

  int new_process = last_empty_fragment;

  // Si hay un fragmento de memoria vacío, se coloca ahí
  if (last_empty_fragment == -1)
  {
    new_process = ++last_assigned;
    List_RS(&Fragments[new_process].heap, frag_size / 2);
  }
  // Si no hay un fragmento de memoria vacío, crear uno nuevo
  else
  {
    List_Init(&Fragments[new_process].heap, frag_size / 2);
  }
  Fragments[new_process].process_pid = process.pid;
  Fragments[new_process].base = last_assigned * frag_size;
  Fragments[new_process].bound = frag_size;
  Fragments[new_process].stack_pointer = frag_size;
  Fragments[new_process].heap_pointer = frag_size / 2;
  // Guardar el proceso como proceso actual
  current_fragment = new_process;
  m_set_owner(Fragments[new_process].base, Fragments[new_process].base + Fragments[new_process].bound);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  //Desvinculando el proceso finalizado de su fragmento de memoria asignado
  for (int i = 0; i <= last_assigned; i++) 
  {
    if (Fragments[i].process_pid == process.pid) 
    {
      m_unset_owner(Fragments[i].base, Fragments[i].base + Fragments[i].bound);
      Fragments[i].process_pid = -1;
      return;
    }
  }
}
