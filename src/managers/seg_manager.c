#include "seg_manager.h"
#include "seg_structs.h"
#include "free_list.h"
#define EXITO 0
#define ERROR 1

#include "stdio.h"

seg_node_t *seg_node_head = NULL;  
seg_node_t *seg_current = NULL;

// // Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) 
{  
  seg_node_clean(seg_node_head);
  seg_current = NULL;
  freelist_init(m_size());
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) 
{
  if (seg_current->proc_mem_info.heap_pos + size < HEAP_SEG_SIZE)
  {
    out->addr = seg_current->proc_mem_info.heap_pos ;
    out->size = size;
    seg_current->proc_mem_info.heap_pos += size + 1;  
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) 
{
  if ( ptr.addr <= seg_current->proc_mem_info.heap_pos )
  {
    if (ptr.addr == seg_current->proc_mem_info.heap_pos - ptr.size - 1)
    {
      seg_current->proc_mem_info.heap_pos -= ptr.size;
    }
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) 
{
  if ( seg_current->proc_mem_info.stack_pos -1 > 0)
  {
    seg_current->proc_mem_info.stack_pos -= 1;
    m_write(seg_current->proc_mem_info.stack_pos + seg_current->proc_mem_info.stack_base, val);
    out->addr = seg_current->proc_mem_info.stack_pos;
    out->size = 1;
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Quita un elemento del stack
int m_seg_pop(byte *out) 
{
  if ( seg_current->proc_mem_info.stack_pos + 1 <= STACK_SEG_SIZE)
  {
    *out = m_read(seg_current->proc_mem_info.stack_base + seg_current->proc_mem_info.stack_pos);
    seg_current->proc_mem_info.stack_pos += 1;
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) 
{
  if (addr <= HEAP_SEG_SIZE)
  {
    *out = m_read(seg_current->proc_mem_info.heap_base + addr);
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) 
{
  if (addr <= HEAP_SEG_SIZE)
  {
    m_write(seg_current->proc_mem_info.heap_base + addr, val);
    return EXITO;
  }
  else
  {
    return ERROR;
  }
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) 
{
  seg_node_t *temp = seg_find_node(process.pid,seg_node_head);

  if(temp != NULL)
  {
    seg_current = temp;
    return;
  }
  seg_current = (seg_node_t *) malloc(sizeof(seg_node_t));
  seg_current->proc_mem_info.pid = process.pid;
  //info del codigo
  seg_current->proc_mem_info.cod_base = find_freelist_node(process.program->size);
  seg_current->proc_mem_info.cod_bounds = process.program->size;
  m_set_owner(seg_current->proc_mem_info.cod_base, seg_current->proc_mem_info.cod_base + seg_current->proc_mem_info.cod_bounds);
  //info del heap
  seg_current->proc_mem_info.heap_base = find_freelist_node( HEAP_SEG_SIZE);
  m_set_owner(seg_current->proc_mem_info.heap_base, seg_current->proc_mem_info.heap_base + HEAP_SEG_SIZE);
  seg_current->proc_mem_info.heap_pos = 0;
  //info del stack
  seg_current->proc_mem_info.stack_base = find_freelist_node(STACK_SEG_SIZE);
  m_set_owner(seg_current->proc_mem_info.stack_base, seg_current->proc_mem_info.stack_base +STACK_SEG_SIZE);
  seg_current->proc_mem_info.stack_pos = STACK_SEG_SIZE;

  seg_current->next = seg_node_head;
  seg_node_head = seg_current;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) 
{
  seg_node_t *temp = seg_node_head;
  seg_node_t *previus = NULL;
  
  while (temp != NULL) 
  {
    if (temp->proc_mem_info.pid == process.pid) 
    {
      if (previus == NULL)
      {
        seg_node_head = temp->next;
      }
      else 
      {
        previus->next = temp->next;
      }

      m_unset_owner(temp->proc_mem_info.cod_base, temp->proc_mem_info.cod_base + temp->proc_mem_info.cod_bounds);
      m_unset_owner(temp->proc_mem_info.heap_base, temp->proc_mem_info.heap_base + HEAP_SEG_SIZE);
      m_unset_owner(temp->proc_mem_info.stack_base, temp->proc_mem_info.stack_base + STACK_SEG_SIZE);
      add_freelist_node(temp->proc_mem_info.cod_base, temp->proc_mem_info.cod_bounds);
      add_freelist_node(temp->proc_mem_info.heap_base, HEAP_SEG_SIZE);
      add_freelist_node(temp->proc_mem_info.stack_base,STACK_SEG_SIZE );
      free(temp);
      return;    
    }
    previus = temp;
    temp = temp->next;
  }
}