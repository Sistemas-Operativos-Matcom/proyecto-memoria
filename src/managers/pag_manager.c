#include "pag_manager.h"
#include "auxiliar_pag/list_pag.h"
#include "../utils.h"
#include "stdio.h"
#include "math.h"
static size_t *pag_table_frame;
static size_t pag_table_frame_c;
static size_t pag_size = 128;
static size_t pag_size_inb = 7; // representa la n de 2^n = pag_size
static pList *process_l;
static process_pag *process_act;
// la pagina actual es process_act->pag_process[process_act->pag_process_c - 1]

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  // process_act = NULL;
  size_t mem = m_size();
  pag_table_frame_c = mem / pag_size;
  pag_table_frame = (size_t *)malloc(pag_table_frame_c * sizeof(size_t));

  process_l = Init_l_pag();
}
// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  if (ptr.size > pag_size)
  {
    size_t addr = ptr.size;
    size_t pag = ptr.addr >> pag_size_inb;
    size_t pos_inpag = ptr.addr - (pag_size * pag);
    while (1)
    {

      // printf("size: %zu paginas: %zu addr: %zu frame: %zu\n", addr, pag, ptr.addr, process_act->pag_process[pag]);
      pag_table_frame[process_act->pag_process[pag]] = 0;
      m_unset_owner(process_act->pag_process[pag] * pag_size, (process_act->pag_process[pag] + 1) * pag_size);
      process_act->pag_process[pag] = 0;
      if (addr >= pag_size)
      {
        addr -= pag_size;
        for (size_t k = 0; k < pag_size; k++)
        {
          process_act->pag_process_free[pag][k] = 0;
        }
      }
      else
      {
        for (size_t k = 0; k < addr; k++)
        {
          process_act->pag_process_free[pag][k] = 0;
        }
        addr = 0;
      }
      pag += 1;
      if (addr == 0)
        return 0;
    }
  }
  else
  {
    size_t pag = ptr.addr >> pag_size_inb;
    size_t pos_inpag = ptr.addr - (pag_size * pag);
    for (size_t i = 0; i < ptr.size; i++)
    {
      if (pos_inpag >= pag_size)
      {
        pos_inpag = 0;
        pag++;
      }
      size_t addr_phy = (process_act->pag_process[pag] * pag_size) + pos_inpag + i;
      process_act->pag_process_free[pag][pos_inpag + i] = 0;
      m_write(addr_phy, 0);
    }

    return 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  if (size > pag_size)
  {
    size_t count = size;
    process_act->pag_process_c += 1;
    size_t pag = size >> pag_size_inb;
    out->addr = (process_act->pag_process_c - 1) * pag_size;
    out->size = size;
    for (size_t i = 0; i < pag_table_frame_c; i++)
    {
      if (pag_table_frame[i] != 1)
      {
        pag_table_frame[i] = 1;
        process_act->pag_process[process_act->pag_process_c - 1] = i;
        m_set_owner(i * pag_size, (i + 1) * pag_size);
        if (count >= pag_size)
        {
          count -= pag_size;
          for (size_t k = 0; k < pag_size; k++)
          {
            process_act->pag_process_free[process_act->pag_process_c - 1][k] = 1;
          }
          process_act->pag_process_c++;
        }
        else
        {
          for (size_t k = 0; k < count; k++)
          {
            process_act->pag_process_free[process_act->pag_process_c - 1][k] = 1;
          }
          count = 0;
        }
        if (count == 0)
          return 0;
      }
    }
    fprintf(stderr, "La memoria está llena.");
    return 1;
  }
  else
  {

    int res = 1;
    for (size_t i = 0; i < process_act->pag_process_c; i++)
    {
      for (size_t j = 0; j < pag_size; j++)
      {
        if (process_act->pag_process_free[i][j] != 1)
        {
          size_t k = 0;
          for (; k < size && ((j + k) < pag_size); k++)
          {
            if (process_act->pag_process_free[i][j + k] == 1)
              break;
          }
          if (k == size)
          {
            out->addr = (i * pag_size) + j;
            out->size = size;
            // printf("size: %zu paginas: %zu addr: %zu\n", size, i, out->addr);
            for (size_t k = 0; k < size; k++)
              process_act->pag_process_free[i][j + k] = 1;
            res = 0;
            return res;
          }
          else
            j += k;
        }
      }
    }
    if (res)
    {
      for (size_t w = 0; w < pag_table_frame_c; w++)
      {
        if (pag_table_frame[w] != 1)
        {
          process_act->pag_process_c += 1;
          pag_table_frame[w] = 1;
          process_act->pag_process[process_act->pag_process_c - 1] = w;
          if (process_act->pag_process_c > pag_table_frame_c)
          {
            fprintf(stderr, "La memoria está llena.");
            return 1;
          }
          size_t addr_phy = (process_act->pag_process[process_act->pag_process_c - 1] * pag_size);

          m_set_owner(addr_phy, addr_phy + pag_size);

          out->size = size;
          out->addr = (process_act->pag_process_c - 1) * pag_size;
          for (size_t k = 0; k < size; k++)
            process_act->pag_process_free[process_act->pag_process_c - 1][k] = 1;
          res = 0;
          return res;
        }
      }
    }

    return res;
  }
}
int m_pag_push(byte val, ptr_t *out)
{
  for (size_t i = 0; i < process_act->pag_process_c; i++)
  {
    for (size_t j = 0; j < pag_size; j++)
    {
      if (process_act->pag_process_free[i][j] != 1)
      {
        process_act->pag_process_free[i][j] = 1;
        size_t addr_phy = (process_act->pag_process[i] * pag_size) + j;
        out->addr = (i * pag_size) + j;
        out->size = 1;
        m_write(addr_phy, val);
        Push_s_pag(process_act->s, out->addr);
        return 0;
      }
    }
  }
  process_act->pag_process_c += 1;
  if (process_act->pag_process_c > pag_table_frame_c)
  {
    fprintf(stderr, "La memoria está llena.");
    return 1;
  }
  process_act->pag_process_free[process_act->pag_process_c - 1][0] = 1;
  size_t addr_phy = (process_act->pag_process[process_act->pag_process_c - 1] * pag_size);
  out->addr = (process_act->pag_process_c - 1) * pag_size;
  out->size = 1;
  m_write(out->addr, val);
  Push_s_pag(process_act->s, out->addr);

  return 0;
}
// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  size_t addr = Pop_s_pag(process_act->s);
  size_t pag = (addr >> pag_size_inb);
  *out = m_read((process_act->pag_process[pag]) * pag_size + (addr - (pag_size * pag)));
  process_act->pag_process_free[pag]--;
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  size_t pag = addr >> pag_size_inb;
  size_t pos_inpag = addr - (pag_size * pag);
  size_t addr_phy = (process_act->pag_process[pag] * pag_size) + pos_inpag;
  *out = m_read(addr_phy);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  size_t pag = addr >> pag_size_inb;
  size_t pos_inpag = addr - (pag_size * pag);
  size_t addr_phy = (process_act->pag_process[pag] * pag_size) + pos_inpag;
  m_write(addr_phy, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{

  /*
    Verifico si el proceso ya esta creado, si sí lo busco y lo pongo como actual.
    Si no está creado lo creo y pongo su código en las páginas libres que sean necesarias.
  */
  int pos = Contains_l_pag(process_l, process.pid);
  if (pos != -1)
  {
    *process_act = process_l->data[pos];
    set_curr_owner(process.pid);
  }
  else
  {
    size_t sizep = process.program->size;
    size_t pag_code_c = process.program->size / pag_size;
    if (process.program->size % pag_size != 0)
      pag_code_c++;
    if (process.program->size == 0)
      pag_code_c = 1;

    process_act = Init_p_pag(process.pid, pag_size, pag_table_frame_c);
    set_curr_owner(process.pid);
    size_t j = 0;
    for (size_t i = 0; i < pag_table_frame_c; i++)
    {
      if (pag_table_frame[i] != 1)
      {
        m_set_owner(i * pag_size, (i + 1) * pag_size);
        pag_table_frame[i] = 1;
        if (sizep >= pag_size)
        {
          sizep -= pag_size;
          for (size_t k = 0; k < pag_size; k++)
          {
            process_act->pag_process_free[j][k] = 1;
          }
        }
        else
        {
          for (size_t k = 0; k < sizep; k++)
          {
            process_act->pag_process_free[j][k] = 1;
          }
        }

        process_act->pag_process[j] = i;
        process_act->pag_process_c++;
        j++;
        if (j == pag_code_c)
          break;
      }
    }
    // printf("test: %zu\n", process_act->pag_process_free[process_act->pag_process_c - 1]);
    if (j != pag_code_c)
    {
      fprintf(stderr, "No hay espacio para el programa");
      exit(1);
    }
    Push_l_pag(process_l, *process_act);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  /*
    Busco el proceso y lo elimino de la lista de procesos.
    Luego libero las páginas que ocupaba.
  */
  set_curr_owner(process.pid);
  int pos = Contains_l_pag(process_l, process.pid);
  if (pos != -1)
  {
    process_pag *p = &process_l->data[pos];
    for (size_t i = 0; i < p->pag_process_c; i++)
    {
      pag_table_frame[p->pag_process[i]] = 0;
      m_unset_owner(p->pag_process[i] * pag_size, (p->pag_process[i] + 1) * pag_size);
    }
    Free_p_pag(p);
    RemovePos_l_pag(process_l, pos);
  }
}
