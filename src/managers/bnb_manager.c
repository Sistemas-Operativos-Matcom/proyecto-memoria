#include "bnb_manager.h"
#include "bnb_structs.h"
#define EXITO 0
#define ERROR 1

#include "stdio.h"


bnb_node_t *node_head = NULL;  
bnb_node_t *current = NULL;
ulong free_position = 0;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) 
{
  bnb_node_clean(node_head);
  current = NULL;
  free_position = 0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) 
{
  if (current->proc_mem_info.heap_pos + size < current->proc_mem_info.bounds - current->proc_mem_info.stack_bounds )
  {
    out->addr = current->proc_mem_info.heap_pos;
    out->size = size;
    current->proc_mem_info.heap_pos += size;  
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) 
{
  if (ptr.addr <= current->proc_mem_info.heap_pos)
  {
    if (ptr.addr == current->proc_mem_info.heap_pos - ptr.size)
    {
      current->proc_mem_info.heap_pos -= ptr.size;
    }
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) 
{
  if (current->proc_mem_info.stack_pos - 1 > current->proc_mem_info.bounds - current->proc_mem_info.stack_bounds){
    current->proc_mem_info.stack_pos -= 1;
    m_write( current->proc_mem_info.base + current->proc_mem_info.stack_pos, val);
    out->addr =  current->proc_mem_info.stack_pos;
    out->size = 1;
    return EXITO;
  }
  else
  {  
    return ERROR;
  }
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) 
{
  if (current->proc_mem_info.stack_pos + 1 <= current->proc_mem_info.bounds)
  {
    *out = m_read(current->proc_mem_info.base + current->proc_mem_info.stack_pos);
    current->proc_mem_info.stack_pos += 1;
    return EXITO;
  }
  else
  {  
    return ERROR;
  }
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) 
{
  if ( current->proc_mem_info.base + current->proc_mem_info.cod_bounds+ addr <= current->proc_mem_info.base + current->proc_mem_info.bounds )
  {
    *out = m_read(current->proc_mem_info.base + addr);
    return EXITO;
  }
  else
  {  
    return ERROR;
  }
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) 
{
  if ( current->proc_mem_info.base + current->proc_mem_info.cod_bounds + addr <= current->proc_mem_info.base + current->proc_mem_info.bounds )
  {
    m_write(current->proc_mem_info.base + addr, val);
    return EXITO;
  }
  else
  {  
    return ERROR;
  }
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) 
{
  bnb_node_t *temp = bnb_find_node(process.pid,node_head);
  
  if(temp == NULL)
  {
    current = bnb_init_node(process,free_position);
    m_set_owner( free_position, free_position + current->proc_mem_info.bounds-1);
    free_position = free_position + current->proc_mem_info.bounds;
    current->next = node_head;
    node_head = current;
  }
  else
  {
    current = temp;
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) 
{
  bnb_node_t *temp = node_head;
  bnb_node_t *previus = NULL;
  
  while (temp != NULL) 
  {
    if (temp->proc_mem_info.pid == process.pid) 
    {
      if (previus == NULL)
      {
        node_head = temp->next;
      }
      else 
      {
        previus->next = temp->next;
      }
      m_unset_owner(temp->proc_mem_info.base, temp->proc_mem_info.base + temp->proc_mem_info.bounds-1);
      free(temp);
      return;    
    }
    previus = temp;
    temp = temp->next;
  }
}
