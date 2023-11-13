#include "pag_manager.h"
#include "pag_structs.h"

#define EXITO 0
#define ERROR 1

#include "stdio.h"

pag_t *pag_control = NULL;
size_t cantPagF = NULL;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) 
{
  if (pag_control != NULL) 
  {
    pag_clear_v(pag_control);
    free(pag_control);
  }
  cantPagF = KB(m_size());

  pag_control= (pag_t *)malloc(cantPagF * sizeof(pag_t));
  pag_init_m(pag_control,cantPagF);
  pag_control->index_proc=-1;
  pag_control->pid_proc= -1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) 
{
  size_t index = pag_find_freemem_a(pag_control,cantPagF);
  if(index == -1) return ERROR;

  out->addr = index * BLOCK_SIZE;
  out->size = 1;
  pag_control->virtual_mem[index] = pag_control->pid_proc;
  pag_control->process[pag_control->index_proc].page_table[0] = index;
  m_set_owner(index * BLOCK_SIZE, (index + 1) * BLOCK_SIZE - 1);

  return EXITO;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) 
{
  size_t i_pf = ptr.addr / BLOCK_SIZE;
  size_t f_pf = (ptr.addr + ptr.size) / BLOCK_SIZE;

  if (f_pf >= cantPagF || ptr.size > pag_control->process[pag_control->index_proc].heap)  return ERROR;

  for (size_t i = i_pf; i < f_pf; i++)
  {
    if (pag_control->virtual_mem[i] != pag_control->pid_proc) return ERROR;
  }

  pag_control->process[pag_control->index_proc].heap -= ptr.size;

  for (size_t i = 0; i < PAGES; i++)
  {
    size_t pf = pag_control->process[pag_control->index_proc].page_table[i];

    if (pf > i_pf && pf <= f_pf)
    {
      m_unset_owner(pf * BLOCK_SIZE, (pf + 1) * BLOCK_SIZE - 1);
      pag_control->process[pag_control->index_proc].page_table[i] = -1;
    }
  }

  return EXITO; 
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) 
{
  size_t size_st = STACK_SIZE - pag_control->process[pag_control->index_proc].stack;
  size_t p;

  if (pag_control->process[pag_control->index_proc].heap + 1 == pag_control->process[pag_control->index_proc].stack) return ERROR;

  if (size_st % BLOCK_SIZE == 0) 
  {
    size_t index = pag_find_freemem_a(pag_control,cantPagF);
    if(index != -1)
    {
      pag_control->virtual_mem[index] = pag_control->pid_proc;
      p = PAGES - (size_t)(size_st / BLOCK_SIZE) - 1;
      pag_control->process[pag_control->index_proc].page_table[p] = index;
      m_set_owner(index * BLOCK_SIZE, (index + 1) * BLOCK_SIZE - 1);
    }
  }

  pag_control->process[pag_control->index_proc].stack -= 1;
  size_st += 1;
  p = PAGES - (size_t)(size_st / BLOCK_SIZE) - 1;
  size_t p_f = pag_control->process[pag_control->index_proc].page_table[p];
  size_t addr = (p_f * BLOCK_SIZE) + (size_st % BLOCK_SIZE);
  m_write(addr, val);
  out->addr = pag_control->process[pag_control->index_proc].stack;
  return EXITO;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) 
{
  if (pag_control->process[pag_control->index_proc].stack == STACK_SIZE) return ERROR;

  size_t size_st = STACK_SIZE - pag_control->process[pag_control->index_proc].stack;
  size_t p = PAGES - (size_t)(size_st / BLOCK_SIZE) - 1;
  size_t p_f = pag_control->process[pag_control->index_proc].page_table[p];
  size_t addr = (p_f * BLOCK_SIZE) + (size_st % BLOCK_SIZE);

  *out = m_read(addr);
  pag_control->process[pag_control->index_proc].stack += 1;

  if (size_st % BLOCK_SIZE == 0) 
  {
    pag_control->virtual_mem[p_f] = -1;
    pag_control->process[pag_control->index_proc].page_table[p] = -1;
    m_unset_owner(p_f * BLOCK_SIZE, (p_f + 1) * BLOCK_SIZE - 1);
  }

  return EXITO;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) 
{
  size_t current = (size_t)(addr / BLOCK_SIZE);

  if (pag_control->virtual_mem[pag_control->index_proc] == pag_control->pid_proc) 
  {
    *out = m_read(addr);
    return EXITO;
  }

  return ERROR; 
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) 
{
  size_t current = (size_t)(addr / BLOCK_SIZE);

  if (pag_control->virtual_mem[current] == pag_control->pid_proc) 
  {
    m_write(addr, val);
    return EXITO;
  }

  return ERROR;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) 
{
  pag_control->pid_proc = process.pid;

  size_t index = pag_find_pidproc_b(pag_control,cantPagF,process.pid);
  if(index != -1)
  {
    pag_control->index_proc = index;
    return;
  }

  index = pag_find_freeproc_c(pag_control,cantPagF);
  
  if (index != -1) 
  {
    pag_control->process[index].asig = 1;
    pag_control->process[index].user_pid = process.pid;
    pag_control->virtual_mem[index] = process.pid;
    pag_control->index_proc = index;
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) 
{
  for (size_t i = 0; i < cantPagF; i++) 
  {
    if (pag_control->process[i].user_pid == process.pid) 
    {
      pag_control->process[i].asig = 0;
      int page_frame;

      for (size_t j = 0; j < PAGES; j++) 
      {
        page_frame = pag_control->process[i].page_table[j];
        if (page_frame != -1) 
        {
          pag_control->virtual_mem[page_frame] = 0;
        }
        pag_control->process[i].page_table[j] = -1;
        m_unset_owner(i * BLOCK_SIZE, (i + 1) * BLOCK_SIZE - 1);
      }

      pag_control->process[i].user_pid = -1;
      pag_control->process[i].heap = 0;
      pag_control->process[i].stack = STACK_SIZE;
    }
  }
}