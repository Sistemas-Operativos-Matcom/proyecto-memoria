#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"
#define __PROCESS_MAX__ 100

int proc_active;
int stack_size;
int bounds;
int *pids;
addr_t *bases;
addr_t *heaps;
addr_t *stacks_pointers;
addr_t *free_list;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) 
{
  bounds = m_size() / __PROCESS_MAX__;
  proc_active = -1;
  stack_size = bounds / 4;
  pids = (int *)malloc(__PROCESS_MAX__ * sizeof(int));
  bases = (addr_t *)malloc(__PROCESS_MAX__ * sizeof(addr_t));
  heaps = (addr_t *)malloc(__PROCESS_MAX__ * sizeof(addr_t));
  stacks_pointers = (addr_t *)malloc(__PROCESS_MAX__ * sizeof(addr_t));
  free_list = (addr_t *)malloc(m_size() * sizeof(addr_t));  

  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    pids[i] = -1;
    bases[i] = i * bounds;
    heaps[i] = 0; 
    stacks_pointers[i] = bounds - 1;
  }

  for (size_t i = 0; i < m_size(); i++)
  {
    free_list[i] = 0;
  }  
}

//Reserva un espacio en el heap de tamanno 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)  
{
  if(proc_active == -1) return 1;

  if(heaps[proc_active] + size < stacks_pointers[proc_active])
  {
    out->addr = heaps[proc_active];
    out->size = size;

    for (addr_t i = bases[proc_active] + heaps[proc_active]; i < bases[proc_active] + heaps[proc_active] + size; i++)
    {
      free_list[i] = 1;
    }
    heaps[proc_active] += size;
    return 0;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)  //guardar de alguna forma los espacios libres
{
  if (ptr.addr + ptr.size > bases[proc_active] + bounds) return 1;

  m_unset_owner(bases[proc_active] + ptr.addr, bases[proc_active] + ptr.addr + ptr.size);

  for (addr_t i = bases[proc_active] + ptr.addr; i < bases[proc_active] + ptr.addr + ptr.size; i++)
  {
    free_list[i] = 0;
  }
  return 0;  
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) 
{
  if (stacks_pointers[proc_active] == bases[proc_active] + (bounds - stack_size)) return 1;

  m_write(bases[proc_active] + stacks_pointers[proc_active], val);
  
  out->addr = stacks_pointers[proc_active];
  out->size = val;

  free_list[bases[proc_active] + stacks_pointers[proc_active]] = 1;
  stacks_pointers[proc_active]-- ;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) 
{
  if(stacks_pointers[proc_active] == bases[proc_active] + bounds - 1) return 1;

  stacks_pointers[proc_active]++;
  *out = m_read(bases[proc_active] + stacks_pointers[proc_active]);
  free_list[bases[proc_active + stacks_pointers[proc_active]]] = 0;   
  return 0;  
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) 
{
  if(bases[proc_active] + addr >= bases[proc_active] + bounds) return 1;
  if(free_list[bases[proc_active] + addr] == 0) return 1;
  *out = m_read(bases[proc_active] + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) 
{
  if(addr > bases[proc_active] + bounds) return 1; //********cambie "addr > bounds" por lo que esta puesto*****
  if(free_list[bases[proc_active] + addr] == 0) return 1;//*******cambie "== 0" por "== 1"********
  free_list[bases[proc_active] + addr] = 1;
  m_write(bases[proc_active] + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  /*if(proc_active == -1)
  {
    proc_active = 0;
    pids[0] = process.pid;
    heaps[0] = process.program->size + 1;
    return;
  }*/
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids[i] == process.pid)
    {
      proc_active = i;
      return;
    }
  }
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids[i] != -1) continue;
    else 
    {
      proc_active = i;
      pids[i] = process.pid;
      m_set_owner(bases[i], bases[i] + bounds);
      //heaps[i] = bases[i] + process.program->size;
      return;
    }   
  }  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) //m_set_owner...
{
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids[i] != process.pid) continue;
    for (addr_t j = bases[i]; j < bases[i] + bounds; j++)
    {
      free_list[j] = 0;
    }
    pids[i] = -1;
    heaps[i] = 0;
    stacks_pointers[i] = bounds - 1;
    m_unset_owner(bases[i], bases[i] + bounds);
    return;    
  }  
}
