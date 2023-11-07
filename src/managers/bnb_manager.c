#include "bnb_manager.h"

#include "stdio.h"

#include "list.h"

#include "../memory.h"

array_list *ramblocks;
array_list *running_procs;
long blocksize = 512;
process_t current_proc;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  int blocks = m_size() / blocksize;
  running_procs = create_list();
  ramblocks = create_list();
  for (int i = 0; i < blocks; i++)
    append(ramblocks, -1);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  for (int i = 0; i < current_proc.size_free->size; i++)
  {

    // printf("%d",current_proc.size_free->array[i]);
    // printf("%d",size);
    if ((size_t)current_proc.size_free->array[i] >= size)
    {
      current_proc.size_free->array[i] -= (int)size;
      out->size = size;
      out->addr = current_proc.start_free->array[i];
      current_proc.start_free->array[i] += (int)size;
      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  for (int i = 0; i < current_proc.start_free->size; i++)
  {
    if (ptr.addr < (size_t)current_proc.start_free->array[i])
    {
      insert(current_proc.start_free, (int)ptr.addr, i);
      insert(current_proc.size_free, (int)ptr.size, i);
    }
  }
  for (int i = 0; i < current_proc.start_free->size - 1; i++)
  {
    if (current_proc.start_free->array[i] + current_proc.size_free->array[i] == current_proc.start_free->array[i + 1])
    {
      current_proc.size_free->array[i] += current_proc.size_free->array[i + 1];
      delete (current_proc.start_free, i + 1);
      delete (current_proc.size_free, i + 1);
      i--;
    }
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  // print_list(current_proc.start_free);
  // print_list(current_proc.size_free);

  if (*current_proc.sp >= 0 && *current_proc.sp < *current_proc.stack_size)
  {
    out->size = 1;
    out->addr = current_proc.blocksused->array[0] * blocksize + *current_proc.stack_base + *current_proc.sp;
    m_write(out->addr, val);
    *current_proc.sp += 1;

    return 0;
  }
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  // print_list(current_proc.start_free);
  // print_list(current_proc.size_free);
  if (*current_proc.sp > 0 && *current_proc.sp <= *current_proc.stack_size)
  {
    *out = m_read((size_t)current_proc.blocksused->array[0] * blocksize + *current_proc.stack_base + *current_proc.sp - 1);
    *current_proc.sp -= 1;
    return 0;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  *out = m_read(addr + (current_proc.blocksused->array[0] * blocksize));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (addr >= current_proc.program->size + *current_proc.sp)
  {
    for (int i = 0; i < current_proc.start_free->size; i++)
    {
      if (current_proc.start_free->array[i] <= (int)addr && current_proc.start_free->array[i] + current_proc.size_free->array[i] > (int)addr)
        return 1;
    }
    m_write(addr + (current_proc.blocksused->array[0] * blocksize), val);
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  // printf("total activos proceso %d \n", running_procs->size);
  // print_list(running_procs);
  current_proc = process;
  // printf("%d", current_proc.size_free->size);
  set_curr_owner(process.pid);
  // Verifico si no esta corriendo
  int running = 0;
  for (int i = 0; i < running_procs->size; i++)
  {
    // printf("%d vs %d\n", running_procs->array[i], process.pid);
    if (running_procs->array[i] == process.pid)
    {
      running = 1;
      break;
    }
  }
  if (!running)
  {
    // printf("nuevo proceso %d \n", current_proc.pid);
    // printf("sizeee %lu \n", current_proc.program->size);
    for (int i = 0; i < ramblocks->size; i++)
    {
      if (ramblocks->array[i] == -1)
      {
        ramblocks->array[i] = process.pid;
        append(process.blocksused, i);
        append(process.start_free, 0);
        append(process.size_free, blocksize);

        // print_list(process.start_free);
        // print_list(process.size_free);

        m_set_owner(i * blocksize, i * blocksize + (blocksize - 1));
        process.start_free->array[0] += process.program->size;
        process.size_free->array[0] -= process.program->size;
        *process.stack_base = process.start_free->array[0];
        *process.stack_size = 64;
        *process.sp = 0;
        process.start_free->array[0] += *process.stack_size;
        process.size_free->array[0] -= *process.stack_size;
        append(running_procs, current_proc.pid);
        // print_list(process.start_free);
        // print_list(process.size_free);
        // print_list(ramblocks);
        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  set_curr_owner(NO_ONWER);
  m_set_owner(process.blocksused->array[0] * blocksize, process.blocksused->array[0] * blocksize + (blocksize - 1));
  for (int i = 0; i < ramblocks->size; i++)
  {
    if (ramblocks->array[i] == process.pid)
    {
      ramblocks->array[i] = -1;
      break;
    }
  }
  for (int i = 0; i < running_procs->size; i++)
  {
    if (running_procs->array[i] == process.pid)
    {
      delete (running_procs, i);
      break;
    }
  }
  print_list(process.size_free);
  print_list(process.start_free);

}
