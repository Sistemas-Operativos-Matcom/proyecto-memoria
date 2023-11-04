#include "bnb_manager.h"

#include "stdio.h"
#include "structures/bnb_seg/memory_manager.h"

#define BNB 1

static memory_manager manager;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  manager = new_memory_manager(m_size());
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  address_space virtual_space = manager->current->virtual_space;

  addr_t virtual_address;
  if (!allocate_as(virtual_space, size, &virtual_address))
    return FALSE;

  out->addr = virtual_address;
  out->size = size;
  return TRUE;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  address_space virtual_space = manager->current->virtual_space;
  if (!deallocate_as(virtual_space, ptr.addr, ptr.addr + ptr.size))
    return FALSE;
  return TRUE;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  pcb _pcb = manager->current;
  address_space virtual_space = _pcb->virtual_space;

  addr_t virtual_address;
  addr_t physical_address;
  if (!push_as(virtual_space, &virtual_address))
    return FALSE;
  if (!translate_virtual_to_physical_stack(_pcb, virtual_address, &physical_address))
    return FALSE;

  m_write(physical_address, val);
  out->addr = virtual_address;
  out->size = 1;

  return TRUE;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  pcb _pcb = manager->current;
  address_space virtual_space = _pcb->virtual_space;

  addr_t virtual_address;
  addr_t physical_address;
  if (!pop_as(virtual_space, &virtual_address))
    return FALSE;
  if (!translate_virtual_to_physical_stack(_pcb, virtual_address, &physical_address))
    return FALSE;

  *out = m_read(physical_address);
  return TRUE;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  pcb _pcb = manager->current;

  addr_t physical_address;
  if (!translate_virtual_to_physical_heap(_pcb, addr, &physical_address))
    return FALSE;

  *out = m_read(physical_address);
  return TRUE;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  pcb _pcb = manager->current;

  addr_t physical_address;
  if (!translate_virtual_to_physical_heap(_pcb, addr, &physical_address))
    return FALSE;

  m_write(physical_address, val);
  return TRUE;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  bool stored = change_process_mm(manager, process, BNB);

  set_curr_owner(manager->current->pid);

  if (stored)
  {
    process_allocation allocation = find_allocation_mm(manager, process.pid);

    if (allocation == NULL)
      fprintf(stderr, "Allocation was not found"), exit(1);

    for (size_t i = 0; i < allocation->max_amount; i++)
    {
      space_block block = allocation->blocks[i];
      if (block != NULL)
      {
        m_set_owner(block->start, block->end);
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  process_allocation allocation = find_allocation_mm(manager, process.pid);

  if (allocation == NULL)
    fprintf(stderr, "Allocation was not found"), exit(1);

  for (size_t i = 0; i < allocation->max_amount; i++)
  {
    space_block block = allocation->blocks[i];

    if (block != NULL)
      m_unset_owner(block->start, block->end);
  }

  remove_process_mm(manager, process);
}
