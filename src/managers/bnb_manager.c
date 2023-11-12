#include "bnb_manager.h"
#include "utils_managers/list_bnb.h"
#include "stdio.h"
#include "../utils.h"


static addr_t heap_start_ptr;//es donde termina code y empieza heap
static process_bnb *curr_process;
static list *procs_list;
static size_t *mem_pages_bnb;//cada posicion es una pagina o bloque, que le corresponde a lo suno a un unico proceso
static size_t pages_amount;
static size_t process_bound = 512;//todos tienen igual bound

void m_bnb_init(int argc, char **argv)
{
  size_t mem_size = m_size();
  pages_amount = mem_size / process_bound;
  mem_pages_bnb = (size_t *)malloc(pages_amount * sizeof(size_t));
  procs_list = Init_list();
  heap_start_ptr = 0;
}

int m_bnb_malloc(size_t size, ptr_t *out)
{
  if (heap_start_ptr + size >= process_bound)
  {
    // no hay memoria suficiente
    fprintf(stderr, "No se dispone de ese tamaño de memoria.");
    return 1;
  }
  for (size_t i = heap_start_ptr; i < process_bound; i++)
  {
    if (curr_process->memory[i] != 1)
    {
      size_t j = 0;
      for (; j < size && ((i + j) < process_bound); j++)
      {
        if (curr_process->memory[i + j] == 1)
          break;
      }
      if (j == size)
      {
        // devuelvo como addr a la dirección de mi memoria virtual.
        out->addr = i;
        out->size = size;
        for (size_t j = 0; j < size; j++)
          curr_process->memory[i + j] = 1;
        return 0;
        break;
      }
      else
        i += j;
    }
  }
  return 1;
}

int m_bnb_free(ptr_t ptr)
{
  if (ptr.addr > process_bound)
  {
    fprintf(stderr, "Dirección incorrecta.");
    return 1;
  }
  for (size_t i = 0; i < ptr.size; i++)
  {
    curr_process->memory[ptr.addr + i] = 0;
    m_write(ptr.addr + curr_process->base, 0);
  }

  return 0;
}

int m_bnb_push(byte val, ptr_t *out)
{
  int result = 1;
  for (size_t i = process_bound - 1; i > 0; i--)
  {
    if (curr_process->memory[i] != 1)
    {
      curr_process->memory[i] = 1;
      result = Push_stack(curr_process->my_stack, i);
      out->addr = i;
      out->size = 1;
      m_write(out->addr + curr_process->base, val);
      return result;
    }
  }
  return result;
}

int m_bnb_pop(byte *out)
{
  size_t last_pos_of_stack = Pop_stack(curr_process->my_stack);
  curr_process->memory[last_pos_of_stack] = 0;//declaramos las ult pos como libre
  *out = m_read(curr_process->base + last_pos_of_stack);//out guarda el puntero al tope de stack
  return 0;
}

int m_bnb_load(addr_t addr, byte *out)
{
  if(addr > process_bound-1)
  {
    return -1;
  }
  *out = m_read(addr + curr_process->base);//se guarda en out valor que corresponde al addr en memoria del proceso
  return 0;
}

int m_bnb_store(addr_t addr, byte val)
{
  if(addr > process_bound-1 || addr <= heap_start_ptr - 1)
  {
    return -1;
  }
  m_write(addr + curr_process->base, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  int pos = bnb_Contains(procs_list, process.pid);
  if (pos != -1)
  {//si llegas aqui proceso ya estaba activo en procs_list
    *curr_process = procs_list->data[pos];
    heap_start_ptr = process.program->size;
    set_curr_owner(process.pid);
  }
  else
  {//llega un proceso nuevo
    for (size_t i = 0; i < pages_amount; i++)
    {
      if (mem_pages_bnb[i] != 1)
      {
        mem_pages_bnb[i] = 1;
        curr_process = Init_proc_bnb(process.pid, process_bound, i);
        set_curr_owner(process.pid);
        m_set_owner(curr_process->base, curr_process->base + process_bound);
        if (process.program->size > process_bound)
        {
          fprintf(stderr, "El size del codigo del programa es mayor que el valor del bound.");
          exit(1);
        }
        for (size_t i = 0; i < process.program->size; i++)
          curr_process->memory[i] = 1;
        // el valor del puntero representa a partir de donde empieza el heap
        heap_start_ptr = process.program->size;
        break;
      }
    }
    bnb_Push(procs_list, *curr_process);//add proceso nuevo a procs_list
  }
}


void m_bnb_on_end_process(process_t process)
{
  int pos = bnb_Contains(procs_list, process.pid);
  if (pos == -1)
  {
    fprintf(stderr, "No se puede eliminar un proceso que no se esta ejecutando.");
    exit(1);
  }
  size_t temp = procs_list->data[pos].base;
  mem_pages_bnb[temp / process_bound] = 0;
  m_unset_owner(temp, temp + process_bound);
  bnb_RemovePos(procs_list, pos);
    if (curr_process->pid == process.pid)
    Free_p_bnb(curr_process);
}