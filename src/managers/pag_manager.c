#include "pag_manager.h"
#include "auxiliar_pag/list_pag.h"
#include "../utils.h"
#include "stdio.h"
static size_t *pag_table_frame;
static size_t pag_table_frame_c;
static size_t pag_size = 64;
static IntList *process_l;
static process_pag *process_act;
// la pagina actual es process_act->pag_process[process_act->pag_process_c - 1]

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  size_t mem = m_size();
  pag_table_frame_c = mem / pag_size;
  pag_table_frame = (size_t *)malloc(pag_table_frame_c * sizeof(size_t));
  process_l = Init_l_pag();
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc2(size_t size, ptr_t *out)
{
  if (size > pag_size)
  {
    fprintf(stderr, "No se puede reservar esa cantidad de memoria de manera contigua.");
    return 1;
  }
}
int m_pag_malloc(size_t size, ptr_t *out)
{
  if (size > pag_size)
  {
    // creo que por como están implementado los test esto no deberia de ser posible
    // el puntero obtiene la direccion de memoria del inicio y luego va haciendo +1 siendo ese el addr
    // que manda para leer desde la memoria fisica
    // si la proxima pagina reservada por el malloc no es continua no tengo forma de saberlo
    // (se pudiera hacer algo por el estilo buscando n cantidad de paginas contiguas)
    // salvo modificando como funciona el addr que recibe mi funcion
    fprintf(stderr, "No se puede reservar esa cantidad de memoria de manera contigua.");
    return 1;
  }
  if (pag_size - process_act->pag_process_free[process_act->pag_process_c - 1] < size)
  {
    process_act->pag_process_c++;
  }
  // process_act->pag_process_free[process_act->pag_process_c - 1] += size;
  int res = 1;
  for (size_t i = 0; i < pag_table_frame_c; i++)
  {
    if (pag_table_frame[i] != 1)
    {
      m_set_owner(i * pag_size, (i + 1) * pag_size);
      pag_table_frame[i] = 1;
      process_act->pag_process[process_act->pag_process_c - 1] = i;
      res = 0;
      break;
    }
  }
  out->addr = (process_act->pag_process[process_act->pag_process_c - 1] * pag_size) + process_act->pag_process_free[process_act->pag_process_c - 1] + 1;
  process_act->pag_process_free[process_act->pag_process_c - 1] += size + 1;
  out->size = size;
  out->pos_page = 0;
  out->page = process_act->pag_process_c - 1;
  return res;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  size_t pag_frame_pos = process_act->pag_process[ptr.page];
  for (size_t i = 0; i < ptr.size; i++)
  {
    m_write(ptr.addr + i, 0);
  }
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  if (pag_size - process_act->pag_process_free[process_act->pag_process_c - 1] >= 1)
  {
    process_act->pag_process_free[process_act->pag_process_c - 1]++;
    out->addr = (process_act->pag_process[process_act->pag_process_c - 1] * pag_size) + process_act->pag_process_free[process_act->pag_process_c - 1];
    out->size = 1;
    out->page = process_act->pag_process_c - 1;
    out->pos_page = process_act->pag_process_free[process_act->pag_process_c - 1];
    m_write(out->addr, val);

    // printf("val: %u push: %zu in page: %zu pospage: %zu: \n", val, out->addr, out->page, out->pos_page);
    // printf("%u\n", m_read(out->addr));
    Push_s_pag(process_act->s, out->page, out->pos_page);
    return 0;
  }
  else
  {
    process_act->pag_process_c++;
    int res = 1;
    for (size_t i = 0; i < pag_table_frame_c; i++)
    {
      if (pag_table_frame[i] != 1)
      {
        m_set_owner(i * pag_size, (i + 1) * pag_size);
        pag_table_frame[i] = 1;
        process_act->pag_process[process_act->pag_process_c - 1] = i;
        res = 0;
        break;
      }
    }
    if (res == 1)
    {
      fprintf(stderr, "No hay espacio para el programa");
      return 1;
    }
    process_act->pag_process_free[process_act->pag_process_c - 1] = 1;
    out->addr = (process_act->pag_process[process_act->pag_process_c - 1] * pag_size) + 1;
    out->size = 1;
    out->page = process_act->pag_process_c - 1;
    out->pos_page = 1;
    // printf("val: %u push: %zu in page: %zu pospage: %zu: \n", val, out->addr, out->page, out->pos_page);
    // Push_s_pag(process_act->s, out->page, out->pos_page);
    return 0;
  }
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  dupla *d = Pop_s_pag(process_act->s);
  *out = m_read((process_act->pag_process[d->pag]) * pag_size + d->pos_pag);
  // printf("val: %u pop: %zu in page: %zu pospage: %zu: \n", *out, (process_act->pag_process[d->pag]) * pag_size + d->pos_pag, process_act->pag_process[d->pag] - 1, d->pos_pag);
  process_act->pag_process_free[d->pag]--;
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  /*
    ?Directamente leo de la memoria física ?
  */
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  /*
    ?Directamente escribo en la memoria física ?
  */
  m_write(addr, val);
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

    process_act = Init_p_pag(process.pid, pag_code_c, pag_table_frame_c);
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
          process_act->pag_process_free[j] = pag_size;
        }
        else
          process_act->pag_process_free[j] = sizep;

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
