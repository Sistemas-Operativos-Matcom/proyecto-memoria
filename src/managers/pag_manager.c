#include "pag_manager.h"
#include "../memory.h"
#include "stdio.h"
#include "stdlib.h"

#define __MAX_COUNT_PAGES__ 100
#define __PROCESS_MAX__ 20
#define __PAGE_LENGTH__ 64
#define __COUNT_INIT_PAGES__ 4

int proc_active_;
int stack_size_;
int *pids_;
int *count_pages_by_procs;
int *free_pages;
int num_actual_page;
addr_t *pages_frames;
addr_t **heaps_;
addr_t *stacks_pointers_;
addr_t *free_list_;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) 
{
  proc_active_ = -1;
  num_actual_page = -1;
  stack_size_ = __PAGE_LENGTH__;
  pids_ = (int *)malloc(__PROCESS_MAX__ * sizeof(int));
  count_pages_by_procs = (int *)malloc(__PROCESS_MAX__ * sizeof(int));
  free_pages = (int*)malloc(__MAX_COUNT_PAGES__ * sizeof(int));
  pages_frames = (addr_t *)malloc(__MAX_COUNT_PAGES__* sizeof(addr_t));
  heaps_ = (addr_t **)malloc(__PROCESS_MAX__ * sizeof(addr_t *));
  stacks_pointers_ = (addr_t *)malloc(__PROCESS_MAX__ * sizeof(addr_t));
  free_list_ = (addr_t *)malloc(m_size() * sizeof(addr_t));  

  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    pids_[i] = -1;
    count_pages_by_procs[i] = 0;
    heaps_[i] = (addr_t *)malloc(__MAX_COUNT_PAGES__ * sizeof(addr_t)); 
    stacks_pointers_[i] = __PAGE_LENGTH__ - 1;
  
    for (size_t j = 0; j < __MAX_COUNT_PAGES__; j++)
    {
      heaps_[i][j] = 0;      
    }
  }
  for (size_t j = 0; j < __MAX_COUNT_PAGES__; j++)
    {
      free_pages[j] = -1;
      pages_frames[j] = j * __PAGE_LENGTH__;
    }
  for (size_t i = 0; i < m_size(); i++)
  {
    free_list_[i] = 0;
  }  
}

//Reserva un espacio en el heap de tamanno 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)  
{
  if(proc_active_ == -1) return 1;

  if(heaps_[proc_active_][num_actual_page] + size < __PAGE_LENGTH__)
  {
    out->addr = heaps_[proc_active_][num_actual_page];
    out->size = size;

    for (addr_t i = pages_frames[num_actual_page] + heaps_[proc_active_][num_actual_page]; i < pages_frames[proc_active_] + heaps_[proc_active_][num_actual_page] + size; i++)
    {
      free_list_[i] = 1;
    }
    heaps_[proc_active_][num_actual_page] += size;
    return 0;
  }
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] != -1) continue;
    heaps_[proc_active_][num_actual_page] = __PAGE_LENGTH__ - 1;
    free_pages[i] = proc_active_;
    num_actual_page = i;
    count_pages_by_procs[proc_active_] ++;

    return m_pag_malloc(size, out); 
  }
  
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] == proc_active_)
    {
      if(ptr.addr + ptr.size > pages_frames[i] && pages_frames[i] + ptr.addr + ptr.size < pages_frames[i] + __PAGE_LENGTH__)
      {
        m_unset_owner(pages_frames[i] + ptr.addr, pages_frames[i] + ptr.addr + ptr.size);
        for (addr_t j = pages_frames[i] + ptr.addr; j < pages_frames[i] + ptr.addr + ptr.size; j++)
        {
          free_list_[j] = 0;
        }
        return 0;          
      }
    }
  }
  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) 
{
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] == proc_active_)
    {
      if (stacks_pointers_[proc_active_] == pages_frames[i]) return 1;
      m_write(pages_frames[i] + stacks_pointers_[proc_active_], val);  
      out->addr = stacks_pointers_[proc_active_];
      out->size = val;
      free_list_[pages_frames[i] + stacks_pointers_[proc_active_]] = 1;
      stacks_pointers_[proc_active_]-- ;
      return 0;
    }
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) 
{
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] == proc_active_) //el stack es el 1er page de un esp de dir
    {
      if(stacks_pointers_[proc_active_] == pages_frames[i] + __PAGE_LENGTH__ - 1) return 1;
      stacks_pointers_[proc_active_]++;
      *out = m_read(pages_frames[i] + stacks_pointers_[proc_active_]);
      free_list_[pages_frames[i] + stacks_pointers_[proc_active_]] = 0;   
      return 0; 
    }
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) 
{
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] == proc_active_)
    {
      if(pages_frames[i] + addr >= pages_frames[i] + __PAGE_LENGTH__) continue;
      if(free_list_[pages_frames[i] + addr] == 0) return 1;
      *out = m_read(pages_frames[i] + addr);
      return 0;
    }
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) 
{
  for (int i = 0; i < __MAX_COUNT_PAGES__; i++)
  {
    if(free_pages[i] == proc_active_)
    {
      if(addr > pages_frames[i] + __PAGE_LENGTH__) continue;
      free_list_[pages_frames[i] + addr] = 1;
      m_write(pages_frames[i] + addr, val);
      return 0;
    }
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] == process.pid)
    {
      proc_active_ = i;
      int actual_count = count_pages_by_procs[proc_active_];
      for (int j = 0; j < __MAX_COUNT_PAGES__; j++)
      {
        if(free_pages[j] == proc_active_)
        {
          if(actual_count == 1)
          {
            num_actual_page = j;
            break;
          }
          actual_count --;
        }
      }
      return;
    }
  }
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] != -1) continue;
  
      proc_active_ = i;
      pids_[i] = process.pid;
    for (int j = 0; j < __MAX_COUNT_PAGES__; j++)
    {
      if(count_pages_by_procs[proc_active_] == 2) break;
      if(free_pages[j] == -1)
      {
        free_pages[j] = proc_active_;
        count_pages_by_procs[proc_active_] ++;
        num_actual_page = j;
        m_set_owner(pages_frames[j], pages_frames[j] + __PAGE_LENGTH__);
      }
    } 
    break;
  } 
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) 
{
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] != process.pid) continue;

    pids_[i] = -1;
    stacks_pointers_[i] = __PAGE_LENGTH__ - 1;

    for (int j = 0; j < __MAX_COUNT_PAGES__; j++)
    {
      if(count_pages_by_procs[i] == 0) break;
      if(free_pages[j] == i)
      {
        heaps_[i][j] = 0;
        m_unset_owner(pages_frames[j], pages_frames[j] + __PAGE_LENGTH__);
        free_pages[j] = -1;
        for (addr_t k = pages_frames[j]; k < pages_frames[j] + __PAGE_LENGTH__; k++)
        {
          free_list_[k] = 0;
        }
        count_pages_by_procs[i] --;
      }
    }       
    return;    
  }  
}
