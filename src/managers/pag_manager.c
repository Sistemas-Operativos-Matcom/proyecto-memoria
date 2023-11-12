#include "pag_manager.h"
#include <math.h>
#include "LinkedList.h"
#include "stdio.h"

size_t PagesMaxProcs = 32;
size_t PagSize = 32;         // size de las paginas
size_t PagAmmount;            // cantidad de paginas
size_t DirectionsSpace; // size del espacio de direcciones
size_t MaxPages;       // cantidad maxima de paginas necesarias para llenar el espacio de direcciones

process_t pag_current_process; // proceso actual
int *PagesPidActive;           // array de procesos activos
int *PagesListFree;            // array de procesos libres

int **linear_page_table;
addr_t *PagesPidStackPointer; //array de stack pointers
LinkedList_t **HeapFreeList; //linked lists

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  DirectionsSpace = m_size() / PagesMaxProcs;
  PagAmmount = m_size() / PagSize;
  MaxPages = (double)DirectionsSpace / (double)PagSize;
  
  PagesPidActive = (int *)malloc(sizeof(int) * PagesMaxProcs);
  PagesListFree = (int *)malloc(sizeof(int) * PagAmmount);

  linear_page_table = (int **)malloc(sizeof(int *) * PagesMaxProcs);
  PagesPidStackPointer = (addr_t *)malloc(sizeof(addr_t) * PagesMaxProcs);
  HeapFreeList = (LinkedList_t **)malloc(sizeof(LinkedList_t *) * PagesMaxProcs);

  for (size_t i = 0; i < PagesMaxProcs; i++)
  {
    PagesPidActive[i] = 0;
    PagesPidStackPointer[i] = -1;
  }

  for (int i = 0; i < PagAmmount; i++)
  {
    PagesListFree[i] = 1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  LinkedList_t *free_space = FindSpace(HeapFreeList[pag_current_process.pid], size);

  if (free_space == NULL)
    return 1;

  out->addr = free_space->start.addr;
  out->size = size;

  size_t k = 0; // indice para buscar las paginas
  for (size_t i = free_space->start.addr / PagSize; i <= (free_space->start.addr + size - 1) / PagSize; i++)
  {
    if (linear_page_table[pag_current_process.pid][i] == -1)
    {
      while (!PagesListFree[k] && k < PagAmmount)
      { // buscar paginas vacias
        k++;
      }

      if (k < PagAmmount)
      {
        linear_page_table[pag_current_process.pid][i] = k;
        m_set_owner(k * PagSize, (k + 1) * PagSize - 1);
      }
      else
      {
        return 1;
      }
    }
  }
  Update(free_space, free_space->start.size - size); // actulizar el espacio libre

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  if (validate_ptr(ptr))
  {
    for (size_t i = (ptr.addr / PagSize) + 1; i <= ((ptr.addr + ptr.size - 1) / PagSize) - 1; i++)
    {
      PagesListFree[linear_page_table[pag_current_process.pid][i]] = 1;
      linear_page_table[pag_current_process.pid][i] = -1;
    }

    LinkedList_t *_node = Find(HeapFreeList[pag_current_process.pid], ptr);

    //(PagSize - 1) - (ptr.addr + ptr.size - 1) % PagSize formula para calcular cuanto falta para llenar la pagina
    if (_node->start.size >= (PagSize - 1) - (ptr.addr + ptr.size - 1) % PagSize)
    {
      PagesListFree[linear_page_table[pag_current_process.pid][((ptr.addr + ptr.size - 1) / PagSize)]] = 1;
      linear_page_table[pag_current_process.pid][((ptr.addr + ptr.size - 1) / PagSize)] = -1;
    }

    if (_node->prev->start.size >= (ptr.addr) % PagSize)
    {
      PagesListFree[linear_page_table[pag_current_process.pid][((ptr.addr) / PagSize)]] = 1;
      linear_page_table[pag_current_process.pid][((ptr.addr) / PagSize)] = -1;
    }

    Insert(HeapFreeList[pag_current_process.pid], ptr);

    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  
  PagesPidStackPointer[pag_current_process.pid]--; // decrementar el SP

  // Stack Overflow
  if (PagesPidStackPointer[pag_current_process.pid] < DirectionsSpace / 2)
  {
    return 1;
  }

  if (PagesPidStackPointer[pag_current_process.pid] % PagSize == PagSize - 1)
  {
    int found = 0;
    for (size_t i = 0; i < PagAmmount; i++)
    {
      if (PagesListFree[i])
      {
        PagesListFree[i] = 0;
        linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] = i;
        m_set_owner(i * PagSize, (i + 1) * PagSize -1);
        found = 1;
        break;
      }
    }
    if (!found)
    {
      return 1;
    }
  }
  m_write(linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] * PagSize + (PagesPidStackPointer[pag_current_process.pid] % PagSize), val);

  out->addr = PagesPidStackPointer[pag_current_process.pid];
  out->size = sizeof(byte);

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  // verificar que el stack no esta vacio
  if (PagesPidStackPointer[pag_current_process.pid] >= DirectionsSpace)
    return 1;

  // devolver memoria
  *out = m_read(linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] * PagSize + (PagesPidStackPointer[pag_current_process.pid] % PagSize));

  if (PagesPidStackPointer[pag_current_process.pid] % PagSize == PagSize - 1)
  {
    m_unset_owner(linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] * PagSize, (linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] + 1) * PagSize - 1);
    PagesListFree[linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize]] = 1;
    linear_page_table[pag_current_process.pid][PagesPidStackPointer[pag_current_process.pid] / PagSize] = -1;
  }

  PagesPidStackPointer[pag_current_process.pid]++; // incrementar el SP

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  if (pag_validate_addr(addr))
  {
    *out = m_read(linear_page_table[pag_current_process.pid][addr / PagSize] * PagSize + (addr % PagSize)); // convertimos a la direccion fisica
    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (pag_validate_addr(addr))
  {
    m_write(linear_page_table[pag_current_process.pid][addr / PagSize] * PagSize + (addr % PagSize), val); // convertimos a la direccion fisica
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  pag_current_process = process;

  if (!PagesPidActive[process.pid])
  {
    // inicializar el proceso
    ptr_t _ptr;
    _ptr.addr = 0;
    _ptr.size = DirectionsSpace / 2;

    HeapFreeList[process.pid] = (LinkedList_t *)malloc(sizeof(LinkedList_t));
    HeapFreeList[process.pid]->prev = NULL;
    HeapFreeList[process.pid]->next = NULL;
    HeapFreeList[process.pid]->start = _ptr;

    linear_page_table[process.pid] = (int *)malloc(sizeof(int) * MaxPages);
    PagesPidStackPointer[process.pid] = DirectionsSpace;
    PagesPidActive[process.pid] = 1;

    for (size_t i = 0; i < MaxPages; i++)
    {
      linear_page_table[process.pid][i] = -1;
    }

    int heap_ready = 0;
    for (size_t i = 0; i < PagAmmount; i++)
    {
      if (PagesListFree[i])
      {
          linear_page_table[process.pid][0] = i;
          heap_ready = 1;
          PagesListFree[i] = 0;
          m_set_owner(i * PagSize, (i + 1) * PagSize - 1);
          break;
      }
    }

   
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  for (size_t i = 0; i < MaxPages; i++)
  {
    if (linear_page_table[process.pid][i] != -1)
    {
      m_unset_owner(linear_page_table[process.pid][i] * PagSize, (linear_page_table[process.pid][i] + 1) * PagSize - 1);
      PagesListFree[linear_page_table[process.pid][i]] = 1;
    }
  }

  FreeList(HeapFreeList[process.pid]);
  PagesPidActive[process.pid] = 0;
  PagesPidStackPointer[process.pid] = -1;
}

int pag_validate_ptr(ptr_t ptr)
{
  if (ptr.addr >= 0 && ptr.addr + ptr.size < DirectionsSpace / 2 && ValidAdddress(HeapFreeList[pag_current_process.pid], ptr))
    return 1;

  return 0;
}

int pag_validate_addr(addr_t addr)
{
  if (addr >= 0 && addr < DirectionsSpace &&
      (!IsFree(HeapFreeList[pag_current_process.pid], addr) || addr > PagesPidStackPointer[pag_current_process.pid]))
    return 1;

  return 0;
}
