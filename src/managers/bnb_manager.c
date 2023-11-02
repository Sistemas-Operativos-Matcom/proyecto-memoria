#include "bnb_manager.h"
#include "stdio.h"
#include "memory.h"

static lista_t *lista;// free list para manejar el espacio libre de la memoria
static size_t base[100000];//array con el base de cada proceso, los indices son los pid de cada proceso
static size_t bound[1000000];//array con el bound de cada proceso, los indices son los pid
static size_t stack[1000000];//guarda la direccion donde empieza el stack de cada proceso
static lista_t listas[100000];//por cada proceso se crea una free list para manejar el espacio
                              //libre del heap
int act_pid;//pid del proceso actual
int act_size;// size del proceso actual

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  for(int i = 0; i < 1000; i++)
  {
    base[i] = 0;
    bound[i] = 0;
    stack[i] = 0;
  }
  iniciar();
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  addr_t a;
  a = r_memory(size, listas[act_pid]);
  if(a == m_size())
  {
    return 1;
  }
  out->addr = a;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int a = l_memory(listas[act_pid], ptr);
  return a;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  if(act_size + 140 + stack[act_pid] >= 700)
  {
    return 1;
  }
  else
  {
    stack[act_pid]++;
    out->addr = base[act_pid] + 699 - stack[act_pid];
    m_write(base[act_pid] + 699 - stack[act_pid], val);
    return 0;
  }
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  if(stack[act_pid] == 0)
  {
    return 1;
  }
  else
  {
    *out = m_read(base[act_pid] + 699 - stack[act_pid]);
    stack[act_pid]--;
    return 0;
  }
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
    for(int i = 0; i < listas[act_pid].actual; i++)
    {
      if(addr >= listas[act_pid].celda[i].addr && addr <= listas[act_pid].celda[i].addr + listas[act_pid].celda[i].tam)
      {
        return 1;
      }
    }
    if(addr >= base[act_pid] + act_size + 140 && addr <= base[act_pid] + 700 - stack[act_pid])
    {
      return 1;
    }
    *out = m_read(base[act_pid] + act_size + addr);
    return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  for(int i = 0; i < listas[act_pid].actual; i++)
  {
    if(addr >= listas[act_pid].celda[i].addr && addr <= listas[act_pid].celda[i].addr + listas[act_pid].celda[i].tam)
    {
      
      return 1;
    }
  }
  m_write(base[act_pid] + act_size + addr, val);
  return 0;  
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  //Revisa que sea un proceso nuevo
  if(base[process.pid] == 0 && bound[process.pid] == 0)
  {
    size_t a = reservar_memoria(700);
    if(a == m_size())
    {
      exit(1);
    }
    lista_t c;
    c.celda = (celda_t *)malloc(100 * sizeof(celda_t));
    c.actual = 1;
    c.size = 100; 
    c.celda[0].addr = process.program->size;
    c.celda[0].tam = 140;
    listas[process.pid] = c;
    base[process.pid] = a+1;
    bound[process.pid] = 700;
    m_set_owner(a, a + 700);
  }
  act_pid = process.pid;
  act_size = process.program->size;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  liberar_memoria(base[process.pid], 700);
  m_unset_owner(base[process.pid], base[process.pid]+700);
}
//inicializa la free list de la memoria
void iniciar()
{
  lista = (lista_t *)malloc(sizeof(lista_t));
  lista->celda = (celda_t *)malloc(100 * sizeof(celda_t));
  lista->actual = 1;
  lista->size = 100; 
  lista->celda[0].addr = 0;
  lista->celda[0].tam = m_size() - 1;
}
//Reserva un espacio de tamaño tam en la free list
addr_t reservar_memoria(size_t tam)
{
  int i = 0;
  int d = 0;
  int c = 0;
  addr_t a = 0;
  while(i < lista->actual)
  {
    if(lista->celda[i].tam >= tam)
    {
      lista->celda[i].tam = lista->celda[i].tam - tam;
      a = lista->celda[i].addr;
      lista->celda[i].addr =lista->celda[i].addr + tam;
      
      
      if(lista->celda[i].tam == 0)
      {
        d = 1;
        c = i;
      }
      i= (int)lista->actual + 1;
    }
    i++;
  }
  if(d == i)
  {
    while(c < lista->actual -  1)
    {
      lista->celda[c] = lista->celda[c+1];
      c++;
    }
    lista->actual--;
  }
  if(a != m_size())
  {
    return a;
  }
  else
  {
    return m_size();
  }
}
//Libera el espacio especificado en  la free list en la direccion dada
void liberar_memoria(addr_t direc, size_t tam)
{
  int i;
  int d = 0;
  int c = 0;
  for(i = 0; i < lista->actual && c == 0; i++)
  {
    if(lista->celda[i].addr < direc)
    {
      d = i;
    }
    else
    {
      c = 1;
    }
  }
  if(lista->actual + 1 > lista->size)
  {
    lista->celda = (celda_t*)realloc(lista->celda, sizeof(celda_t) * lista->size * 2);
    lista->size = lista->size*2;
  }
  lista->actual++;
  d++;
  i = (int)lista->actual;
  while(i > d)
  {
    lista->celda[i].addr = lista->celda[i-1].addr;
    lista->celda[i].tam = lista->celda[i-1].tam;
    i--;
  }
  lista->celda[d].addr = direc;
  lista->celda[d].tam = tam;
}
//Toma la free list del heap de un proceso y reserva un espacio dado
//si no es posible devuelve un valor igual al size de la memoria
addr_t r_memory(size_t tam, lista_t l)
{
  int i = 0;
  int d = 0;
  int c = 0;
  addr_t a = m_size();
  while(i < l.actual)
  {
    
    if(l.celda[i].tam >= tam)
    {
      l.celda[i].tam = l.celda[i].tam - tam;
      a = l.celda[i].addr;
      l.celda[i].addr = l.celda[i].addr + tam;
      if(l.celda[i].tam == 0)
      {
        d = 1;
        c = i;
      }
      i= (int)l.actual + 1;
    }
    i++;
  }
  if(d == 1)
  {
    while(c < l.actual -  1)
    {
      l.celda[c] = l.celda[c+1];
      c++;
    }
    l.actual--;
  }
  int e = (int)a;

  if(a != m_size())
  {

    return a;
  }
  else
  {
    return m_size();
  }
}
// Dada una free list de un heap de un proceso libera en la direccion dada el espacio especificado
int l_memory(lista_t l, ptr_t ptr)
{
  int i;
  int d = 0;
  int c = 0;
  for(i = 0; i < l.actual && c == 0; i++)
  {
    if(l.celda[i].addr <= ptr.addr)
    {
      d = i;
    }
    else
    {
      c = 1;
    }
  }
  if(l.celda[d].addr == ptr.addr)
  {
    return 1;
  }
  if(l.celda[d].addr + l.celda[d].tam > ptr.addr)
  {
    return 1;
  }
  if(d < l.actual-1)
  {
    if(ptr.addr + ptr.size > l.celda[d+1].addr)
    {
      return 1;
    }
  }
  if(l.actual + 1 > l.size)
  {
    l.celda = (celda_t*)realloc(lista->celda, sizeof(celda_t) * lista->size * 2);
    l.size = l.size*2;
  }
  l.actual++;
  d++;
  i = (int)l.actual;
  while(i > d)
  {
    l.celda[i].addr = l.celda[i-1].addr;
    l.celda[i].tam = l.celda[i-1].tam;
    i--;
  }
  l.celda[d].addr = ptr.addr;
  l.celda[d].tam = ptr.size;
  return 0;
}