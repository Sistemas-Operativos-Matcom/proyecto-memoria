#include "bnb_manager.h"
#include "auxiliar/list.h"
#include "stdio.h"
// #include "auxiliar/stack.h"
#include "../utils.h"
/*
Si es necesario modificar la estructura `ptr_t` pueden hacerlo, solo no deben
cambiar su nombre ni eliminar el campo `addr`.
*/
static addr_t puntero;
static process_bb *process_act;
static IntList *l;
static size_t *virtual_mem;
static size_t virtual_mem_c;
static size_t bb_value = 512;

/*
  Se llama cada vez que se inicia un caso de prueba. Recibe como parámetros la
  cantidad de argumentos y los argumentos con el que se ejecutó el simulador
  Debes tener en cuenta reinicializar aquellas estructuras globales extras que
  utilices en caso de ser necesario.
*/
void m_bnb_init(int argc, char **argv)
{
  size_t mem = m_size();
  virtual_mem_c = mem / bb_value;
  virtual_mem = (size_t *)malloc(virtual_mem_c * sizeof(size_t));
  l = Init_l();
  puntero = 0;
}

/*
  Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
  inicio del espacio reservado.
  Reserva un espacio de tamaño `size` en la memoria y modificar el puntero
  `out` con la dirección al espacio reservado. Esta función debe devolver 0 si
  la operación se realizó correctamente, 1 en caso contrario.
*/
int m_bnb_malloc(size_t size, ptr_t *out)
{
  /*
    Escribe como ocupada la memoria virtual que se encontro libre luego de la reservada por
    el codigo del programa.
  */
  for (size_t i = puntero; i < bb_value; i++)
  {
    if (process_act->memory[i] != 1)
    {
      if (i + size >= bb_value)
      {
        // no hay memoria suficiente
        fprintf(stderr, "No se dispone de ese tamaño de memoria.");
        return 1;
      }
      out->addr = process_act->base + i;
      out->size = size;
      for (size_t j = 0; j < size; j++)
      {
        process_act->memory[i + j] = 1;
      }
      return 0;
      break;
    }
  }
  return 0;
}
/*
  Libera el espacio de memoria al que apunta `ptr`. Esta función debe devolver 0 si
  la operación se realizó correctamente, 1 en caso contrario.
*/
int m_bnb_free(ptr_t ptr)
{
  /*
    Para liberar escribe en el ptr.addr el valor 0 y lo pone como posible para escribir en la memoria
    virtual del programa al que le corresponde esa dirección
    El proceso anterior desde ptr.addr hasta ptr.addr + ptr.size
  */
  if (ptr.addr < process_act->base || ptr.addr > process_act->base + bb_value)
  {
    fprintf(stderr, "La dirección que desea liberar no corresponde al proceso seleccionado actualmente.");
    return 1;
  }
  size_t pos = ptr.addr - process_act->base;
  for (size_t i = 0; i < ptr.size; i++)
  {
    process_act->memory[pos + i] = 0;
    m_write(ptr.addr + i, 0);
  }

  return 0;
}

/*
  Agrega un valor `val` al stack y modifica el puntero `out` con la dirección
  donde se almacenó el valor.  Esta función debe devolver 0 si la operación se
  realizó correctamente, 1 en caso contrario.
*/
int m_bnb_push(byte val, ptr_t *out)
{
  /*
    Empiezo a buscar desde la última posición de memoria virtual del programa hacia arriba la primera
    posición libre, luego de encontrarla meto al stack esa posición y escribo el valor en esa posición
    de memoria.
  */
  int res = 1;
  for (size_t i = bb_value - 1; i > 0; i--)
  {
    if (process_act->memory[i] != 1)
    {
      process_act->memory[i] = 1;
      res = Push_s(process_act->s, i);
      out->addr = process_act->base + i;
      out->size = 1;
      m_write(out->addr, val);
      break;
    }
  }
  return res;
}
/*
  Saca un elemento del stack y lo almacena en la variable `out` Esta función
  debe devolver 0 si la operación se realizó correctamente, 1 en caso
  contrario.
*/
int m_bnb_pop(byte *out)
{
  /*
  Al hacer pop al stack obtengo la posición de la memoria virtual donde guarde el último valor que
  hice push al stack, por lo tanto libero esa posición de la virtual y la leo y devuelvo de la física.
  */
  size_t pos = Pop_s(process_act->s);
  process_act->memory[pos] = 0;
  *out = m_read(process_act->base + pos);
  // printf("\nvalor del stack: %d\n", *out);
  return 0;
}

/*
  Lee lo que hay en la dirección `addr` y lo almacena el valor en la variable
  `out` Esta función debe devolver 0 si la operación se realizó correctamente,
  1 en caso contrario.
*/

int m_bnb_load(addr_t addr, byte *out)
{
  /*
    ?Directamente leo desde la memoria física ?
  */
  *out = m_read(addr);
  return 0;
}

/*
  Agrega un valor `val` al heap en la dirección `addr`.  Esta función debe
  devolver 0 si la operación se realizó correctamente, 1 en caso contrario.
*/
int m_bnb_store(addr_t addr, byte val)
{
  /*
    ?Directamente escribo en la memoria física ?
  */
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  /*
    Verifico si el proceso ya esta creado, si sí lo busco y lo pongo como actual.
    Si no está creado lo creo y pongo al inicio de su memoria virtual el size del código del programa,
    luego lo guardo en la lista de programas.
  */
  int pos = Contains_l(l, process.pid);
  if (pos != -1)
  {
    *process_act = l->data[pos];
    set_curr_owner(process.pid);
  }
  else
  {
    for (size_t i = 0; i < virtual_mem_c; i++)
    {
      if (virtual_mem[i] != 1)
      {
        virtual_mem[i] = 1;
        process_act = Init_p(process.pid, bb_value, i);
        set_curr_owner(process.pid);
        m_set_owner(process_act->base, process_act->base + bb_value);
        if (process.program->size > bb_value)
        {
          fprintf(stderr, "El size del codigo del programa es mayor que la memoria.");
          exit(1);
        }
        for (size_t i = 0; i < process.program->size; i++)
        {
          process_act->memory[i] = 1;
        }
        puntero = process.program->size;
        break;
      }
    }

    Push_l(l, *process_act);
  }
}
/*
  Se ejecuta cada vez que termina la ejecución del proceso `process`.
*/
void m_bnb_on_end_process(process_t process)
{
  /*
  Libera un proceso así como toda su memoria correspondiente;
  */
  if (process_act->pid == process.pid)
    Free_p(process_act);

  int pos = Contains_l(l, process.pid);
  if (pos == -1)
  {
    fprintf(stderr, "El proceso no existe.");
    exit(1);
  }
  size_t temp = l->data[pos].base;
  virtual_mem[temp / bb_value] = 0;
  m_unset_owner(temp, temp + bb_value);

  RemovePos_l(l, pos);
}