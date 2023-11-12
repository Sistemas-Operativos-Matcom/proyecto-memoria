#include "bnb_manager.h"

#include "LinkedList.h"
#include "stdio.h"

int MaxProcs = 32;
int bound;
int *ListFree; // array de procesos libres
int *PidActive;//array de procesos activos

process_t CurrProcs; // proceso actual

addr_t *PidBase; //array de bases
addr_t *PidStackPointer; //array de stack pointers
LinkedList_t **PidHeapListFree; //linked lists

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  PidBase = (addr_t *)malloc(sizeof(addr_t) * MaxProcs);
  PidStackPointer = (addr_t *)malloc(sizeof(addr_t) * MaxProcs);
  ListFree = (int *)malloc(sizeof(int) * MaxProcs);
  PidActive = (int *)malloc(sizeof(int) * MaxProcs);
  PidHeapListFree = (LinkedList_t **)malloc(sizeof(LinkedList_t *) * MaxProcs);

  bound = m_size() / MaxProcs;
  
  for (int i = 0; i < MaxProcs; i++)
  {
    PidBase[i] = -1;
    ListFree[i] = 1;
    PidActive[i] = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  LinkedList_t *free_space = FindSpace(PidHeapListFree[CurrProcs.pid], size);

  if (free_space == NULL)
    return 1;

  
  out->addr = free_space->start.addr;
  out->size = size;

  Update(free_space, free_space->start.size - size); // actulizar el espacio libre

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  if (validate_ptr(ptr))
  {
    Insert(PidHeapListFree[CurrProcs.pid], ptr);
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{  
  PidStackPointer[CurrProcs.pid]--; // decrementar el SP

  // Stack Overflow
  if (PidStackPointer[CurrProcs.pid] < PidBase[CurrProcs.pid] + bound / 2)
  {
    return 1;
  }

  m_write(PidStackPointer[CurrProcs.pid], val);

  
  out->addr = PidStackPointer[CurrProcs.pid];
  out->size = sizeof(byte);

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  // verificar que el stack no esta vacio
  if (PidStackPointer[CurrProcs.pid] >= PidBase[CurrProcs.pid] + bound)
    return 1;

  // devolver memoria
  *out = m_read(PidStackPointer[CurrProcs.pid]);

  PidStackPointer[CurrProcs.pid]++; // incrementar el SP

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if (validate_addr(addr))
  {
    *out = m_read(addr); 

    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (validate_addr(addr))
  {
    m_write(addr, val);
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  CurrProcs = process;

  if (!PidActive[process.pid])
  { // inicializar el proceso
    for (int i = 0; i < MaxProcs; i++)
    {
      if (ListFree[i])
      {
        PidBase[process.pid] = i * bound;

        PidStackPointer[process.pid] = (addr_t)((i + 1) * bound);
        ListFree[i] = 0;

        m_set_owner(PidBase[process.pid], PidBase[process.pid] + bound - 1);
        break;
      }
    }

    //crear el heap
    ptr_t _ptr;
    _ptr.addr = PidBase[process.pid];
    _ptr.size = bound / 2;
    
    PidHeapListFree[process.pid] = (LinkedList_t *)malloc(sizeof(LinkedList_t));
    PidHeapListFree[process.pid]->prev = NULL;
    PidHeapListFree[process.pid]->next = NULL;
    PidHeapListFree[process.pid]->start = _ptr;

    PidActive[process.pid] = 1;

  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  PidActive[process.pid] = 0;
  ListFree[process.pid] = 1;
  m_unset_owner(PidBase[process.pid], PidBase[process.pid] + bound - 1);
  PidBase[process.pid] = -1; 
  FreeList(PidHeapListFree[process.pid]); 
  PidStackPointer[process.pid] = -1;
}

int validate_ptr(ptr_t ptr)
{
  if (ptr.addr >= PidBase[CurrProcs.pid] && ptr.addr + ptr.size < PidBase[CurrProcs.pid] + (bound / 2) && ValidAdddress(PidHeapListFree[CurrProcs.pid], ptr))
    return 1;

  return 0;
}

int validate_addr(addr_t addr)
{
  if (addr >= PidBase[CurrProcs.pid] && addr < PidBase[CurrProcs.pid] + (bound / 2) && !IsFree(PidHeapListFree[CurrProcs.pid], addr))
    return 1;

  return 0;
}