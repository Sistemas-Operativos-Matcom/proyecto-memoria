#include "seg_manager.h"

#include "stdio.h"
int size;
int stackp;
int instrp;
int curr_pid;
int heap_man[2048];

int find(int start, int end)
{
  for (int i = start; i < end; i++)
  {
    if (heap_man[i] == curr_pid)
    {
      return i;
    }
  }
  return -1;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
  for (size_t i = 0; i < m_size(); i++)
  {
    heap_man[i] = -1;
  }
  stackp = m_size() - 1;
  instrp = 1;
}

// Reserva un espacio en el heap_man de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
  if(curr_pid == -1) return 1;
  if (instrp + (int)size > stackp)
    return 1;
  for (size_t i = 0; i < size; i++)
  {
    heap_man[i + instrp] = curr_pid;
  }
  out->addr = instrp;
  instrp += size;
  m_set_owner(instrp - size, instrp);
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
  for (size_t i = 0; i < m_size(); i++)
  {
    if (heap_man[i] == curr_pid){
      heap_man[i]=-1;
      //m_unset_owner(i,i);
      
    };
  }
  return 0;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
  if (stackp == instrp + 1)
    return 1;
  else
  {
    m_set_owner(stackp, stackp);
    heap_man[stackp] = curr_pid;
    m_write(stackp, val);
    stackp--;
    out->addr = stackp + 1;
    return 0;
  }
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
  if (stackp == (int)m_size() - 1)
    return 1;
  int pos = find(instrp, m_size());
  if (pos != -1)
  {
    *out = m_read(pos);
    heap_man[pos] = -1;
    m_unset_owner(pos,pos);
    return 0;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  if ((int)addr< instrp && heap_man[addr] == curr_pid) 
  {
    *out = m_read(addr);
    return 0;
  }
  return 1;
  
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
  if ((int)addr< instrp && heap_man[addr] == curr_pid)
  {
    m_write(addr,val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
  curr_pid= process.pid;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
  
    
      m_unset_owner(0,m_size()-1);
  
}

