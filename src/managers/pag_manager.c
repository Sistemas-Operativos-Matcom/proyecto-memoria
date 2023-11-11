#include "pag_manager.h"
#include <math.h>
#include "linked_list.h"
#include "stdio.h"

size_t pag_max_amount_of_process = 32;
size_t page_size = 32;         // definirlo en potencias de dos para aprovechar el offset
size_t page_amount;            // cantidad de paginas en la ram
size_t espacio_de_direcciones; // cambiar el nombre; averigaur como hacerlo multiplo de 2
size_t max_page_process;       // cantidad maxima de paginas necesarias para contener todo el espacio de direcciones

process_t pag_current_process; // proceso del contexto actual
int *pag_active_pid;           // aqui guardo que procesos estan corriendo en la suimulacion
int *pag_free_list;            // que paginas de la memoria estan en uso y cuales no

int **linear_page_table;
addr_t *pag_stackPointer_by_pid;
linked_list_t **pag_heap_free_list_by_pid;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  espacio_de_direcciones = m_size() / pag_max_amount_of_process; // esto lo hago asi por facilismo pero teoricamente deberia definirlo a partir del tamaño de pagina
  page_amount = m_size() / page_size;
  max_page_process = (double)espacio_de_direcciones / (double)page_size;
  
  pag_active_pid = (int *)malloc(sizeof(int) * pag_max_amount_of_process);
  pag_free_list = (int *)malloc(sizeof(int) * page_amount);

  linear_page_table = (int **)malloc(sizeof(int *) * pag_max_amount_of_process);
  pag_stackPointer_by_pid = (addr_t *)malloc(sizeof(addr_t) * pag_max_amount_of_process);
  pag_heap_free_list_by_pid = (linked_list_t **)malloc(sizeof(linked_list_t *) * pag_max_amount_of_process);

  for (size_t i = 0; i < pag_max_amount_of_process; i++)
  {
    pag_active_pid[i] = 0;
    pag_stackPointer_by_pid[i] = -1;
  }

  for (int i = 0; i < page_amount; i++)
  {
    pag_free_list[i] = 1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  linked_list_t *free_space = find_fit(pag_heap_free_list_by_pid[pag_current_process.pid], size);

  if (free_space == NULL)
    return 1; // sino hay epacio libre no se pude reservar por tanto da error

  // generar el puntero out
  out->addr = free_space->start.addr;
  out->size = size;

  size_t j = 0; // iterador para el bucle while que me permite buscar los pages en o(n)
  for (size_t i = free_space->start.addr / page_size; i <= (free_space->start.addr + size - 1) / page_size; i++)
  {
    /*FILE *f = fopen("a.txt", "r+");
    fprintf(f, "i: %d\n", linear_page_table[pag_current_process.pid][i]);
    fclose(f);*/
    if (linear_page_table[pag_current_process.pid][i] == -1)
    {
      while (!pag_free_list[j] && j < page_amount)
      { // buscamos paginas vacias
        j++;
      }

      if (j < page_amount)
      {
        linear_page_table[pag_current_process.pid][i] = j;
        m_set_owner(j * page_size, (j + 1) * page_size - 1);
      }
      else
      {
        return 1; // no hay page disponibles
      }
    }
  }
  update(free_space, free_space->start.size - size); // actulizar el espacio libre

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  if (validate_ptr(ptr))
  {
    // free a los page que estan completamente contenidos en el espacio que estaba reservado los page inicio y fin pueden tener conflico para liberarlos
    for (size_t i = (ptr.addr / page_size) + 1; i <= ((ptr.addr + ptr.size - 1) / page_size) - 1; i++)
    {
      pag_free_list[linear_page_table[pag_current_process.pid][i]] = 1;
      linear_page_table[pag_current_process.pid][i] = -1;
    }

    linked_list_t *_node = find(pag_heap_free_list_by_pid[pag_current_process.pid], ptr);

    //(page_size - 1) - (ptr.addr + ptr.size - 1) % page_size formula para calcular cuanto falta para llenar la pagina
    if (_node->start.size >= (page_size - 1) - (ptr.addr + ptr.size - 1) % page_size)
    {
      pag_free_list[linear_page_table[pag_current_process.pid][((ptr.addr + ptr.size - 1) / page_size)]] = 1;
      linear_page_table[pag_current_process.pid][((ptr.addr + ptr.size - 1) / page_size)] = -1;
    }

    if (_node->prev->start.size >= (ptr.addr) % page_size)
    {
      pag_free_list[linear_page_table[pag_current_process.pid][((ptr.addr) / page_size)]] = 1;
      linear_page_table[pag_current_process.pid][((ptr.addr) / page_size)] = -1;
    }

    insert(pag_heap_free_list_by_pid[pag_current_process.pid], ptr);

    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  // agregar a memoria q ni idea
  pag_stackPointer_by_pid[pag_current_process.pid]--; // aumentar el stackpointer

  // stackoverflow
  if (pag_stackPointer_by_pid[pag_current_process.pid] < espacio_de_direcciones / 2)
  {
    return 1;
  }

  if (pag_stackPointer_by_pid[pag_current_process.pid] % page_size == page_size - 1)
  {
    int found = 0;
    for (size_t i = 0; i < page_amount; i++)
    {
      if (pag_free_list[i])
      {
        pag_free_list[i] = 0;
        linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] = i;
        m_set_owner(i * page_size, (i + 1) * page_size -1);
        //printf("ownerShip push ");
        found = 1;
        break;
      }
    }
    if (!found)
    {
      return 1; // no hay espacio en la memoria pero no es stack overflow otra vez tema discoduro
    }
  }
  //printf("addr: %d", pag_stackPointer_by_pid[pag_current_process.pid] / page_size);
  m_write(linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] * page_size + (pag_stackPointer_by_pid[pag_current_process.pid] % page_size), val);

  // verificar que no hubo fallo al aumentar el stackpointer
  out->addr = pag_stackPointer_by_pid[pag_current_process.pid];
  out->size = sizeof(byte);

  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  // verificar que el stack no esta vacio
  if (pag_stackPointer_by_pid[pag_current_process.pid] >= espacio_de_direcciones)
    return 1;

  // devolver memoria
  *out = m_read(linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] * page_size + (pag_stackPointer_by_pid[pag_current_process.pid] % page_size));

  if (pag_stackPointer_by_pid[pag_current_process.pid] % page_size == page_size - 1)
  {
    m_unset_owner(linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] * page_size, (linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] + 1) * page_size - 1);
    pag_free_list[linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size]] = 1;
    linear_page_table[pag_current_process.pid][pag_stackPointer_by_pid[pag_current_process.pid] / page_size] = -1;
  }

  pag_stackPointer_by_pid[pag_current_process.pid]++; // disminuir el stackpointer

  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  if (pag_validate_addr(addr))
  {
    *out = m_read(linear_page_table[pag_current_process.pid][addr / page_size] * page_size + (addr % page_size)); // convertimos a la direccion fisica
    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  /*FILE *f = fopen("a.txt", "r+");
  fprintf(f, "addr: %d\n", addr);
  fprintf(f, "valid: %d\n", pag_validate_addr(addr));
  fclose(f);*/
  if (pag_validate_addr(addr))
  {
    m_write(linear_page_table[pag_current_process.pid][addr / page_size] * page_size + (addr % page_size), val); // convertimos a la direccion fisica
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  pag_current_process = process;

  if (!pag_active_pid[process.pid])
  { // inicializar el proceso por primera vez
    // al inicializar el proceso e heap es un solo segmento con todo el tamaño del heap
    ptr_t _ptr;
    _ptr.addr = 0;
    _ptr.size = espacio_de_direcciones / 2; // duda aqui

    pag_heap_free_list_by_pid[process.pid] = (linked_list_t *)malloc(sizeof(linked_list_t));
    pag_heap_free_list_by_pid[process.pid]->prev = NULL;
    pag_heap_free_list_by_pid[process.pid]->next = NULL;
    pag_heap_free_list_by_pid[process.pid]->start = _ptr;

    linear_page_table[process.pid] = (int *)malloc(sizeof(int) * max_page_process);
    pag_stackPointer_by_pid[process.pid] = espacio_de_direcciones;
    pag_active_pid[process.pid] = 1;

    for (size_t i = 0; i < max_page_process; i++)
    {
      linear_page_table[process.pid][i] = -1;
    }

    int heap_ready = 0;
    for (size_t i = 0; i < page_amount; i++)
    {
      if (pag_free_list[i])
      {
          // este es el heap
          linear_page_table[process.pid][0] = i;
          heap_ready = 1;
          pag_free_list[i] = 0;
          m_set_owner(i * page_size, (i + 1) * page_size - 1);
          //printf("ownerShip ctx switch ");
          break;
      }
    }

    // aqui deberia haber un error sino se puede inicializar el proceso pero asumo q los casos no serompen por qno hay disco duro
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  for (size_t i = 0; i < max_page_process; i++)
  {
    if (linear_page_table[process.pid][i] != -1)
    {
      m_unset_owner(linear_page_table[process.pid][i] * page_size, (linear_page_table[process.pid][i] + 1) * page_size - 1);
      pag_free_list[linear_page_table[process.pid][i]] = 1;
    }
  }

  free_entire_list(pag_heap_free_list_by_pid[process.pid]);
  pag_active_pid[process.pid] = 0;
  pag_stackPointer_by_pid[process.pid] = -1;
}

int pag_validate_ptr(ptr_t ptr)
{
  if (ptr.addr >= 0 && ptr.addr + ptr.size < espacio_de_direcciones / 2 && is_valid_ptr(pag_heap_free_list_by_pid[pag_current_process.pid], ptr))
    return 1;

  return 0;
}

// aqui hay que ver lo de q pudo sobre escribir el stacks
// se arreegla con un flag
int pag_validate_addr(addr_t addr)
{
  FILE *f = fopen("a.txt", "r+");
  fprintf(f, "addr: %d\n", addr);
  fclose(f);
  if (addr >= 0 && addr < espacio_de_direcciones &&
      (!is_free(pag_heap_free_list_by_pid[pag_current_process.pid], addr) || addr > pag_stackPointer_by_pid[pag_current_process.pid]))
    return 1;

  return 0;
}

// puedo asumir tamaño ram potencia de 2