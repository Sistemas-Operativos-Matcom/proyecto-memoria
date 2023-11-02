#include "pag_manager.h"

#include "stdio.h"

#include "memory.h"

static list_t *lista;// free list para manejar el espacio libre de la memoria
static proceso_t proceso[100000];// array de proceso, los indices son los pid de cada proceso
static int act_pid;// pid del proceso actual

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  init();
  proceso_t c;
  c.act_stack = -1;
  c.pid = -1;
  for(int a = 0; a < 7; a++)
  {
    c.codigo[a] = m_size();
  }
  for(int i = 0; i < 100000; i++)
  {
    proceso[i] = c;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  addr_t a;
  int i = 0;
  size_t c = size;
  int d = 0;
  while(i < 3 && c > 0)
  {
    if(proceso[act_pid].heap[i] == m_size())
    {
      addr_t b = reservar_memory(50);
      proceso[act_pid].heap[i] = b;
      list_t c;
      c.celda = (casilla_t*)malloc(100 * sizeof(casilla_t));
      c.actual = 1;
      c.size = 100;
      c.celda[0].addr = b;
      c.celda[0].tam = 50;
      proceso[act_pid].heapes[i] = c;
      m_set_owner(b, b + 50);
    }
    if(c < 50){
      a = res_memory(c, proceso[act_pid].heapes[i]);
      if(a == m_size()){
        return 1;
      }
      if(d == 0)
      {
        d = 1;
        out->addr = a;
      }
      c = 0;
    }
    if(c >= 50){
      a = res_memory(49, proceso[act_pid].heapes[i]);
      c = c - 50;
      if(a == m_size()){
        return 1;
      }
      if(d == 0)
      {
        d = 1;
        out->addr = a;
      }
    }
    i++;
  }
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  int a;
  for(int i = 0; i < 3; i++)
  {
    a = lib_memory(proceso[act_pid].heapes[i], ptr);
    if(a == 0)
    {
      return a;
    }
  }
  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if(proceso[act_pid].stack[proceso[act_pid].act_stack] == m_size())
  {
    addr_t a = reservar_memory(50);
    proceso[act_pid].stack[proceso[act_pid].act_stack] = a;
    proceso[act_pid].dir_stack = 0;
    m_set_owner(a, a + 50);
  }
  if(proceso[act_pid].act_stack < 2 && proceso[act_pid].dir_stack == 49)
  {
    proceso[act_pid].act_stack++;
    proceso[act_pid].dir_stack = 0;
    addr_t c = reservar_memory(50);
    proceso[act_pid].stack[proceso[act_pid].act_stack] = c;
    m_set_owner(c, c + 50);
  }
  if(proceso[act_pid].act_stack == 2 && proceso[act_pid].dir_stack == 49)
  {
    return 1;
  }
  m_write(proceso[act_pid].stack[proceso[act_pid].act_stack] + proceso[act_pid].dir_stack, val);
  out->addr = proceso[act_pid].heap[proceso[act_pid].act_stack] + proceso[act_pid].dir_stack;
  proceso[act_pid].dir_stack++;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if(proceso[act_pid].act_stack == 0 && proceso[act_pid].dir_stack == 0)
  {
    return 1;
  }
  if(proceso[act_pid].dir_stack == 0)
  {
    proceso[act_pid].act_stack--;
  }
  else
  {
    proceso[act_pid].dir_stack--;
  }
  *out = m_read(proceso[act_pid].stack[proceso[act_pid].act_stack] + proceso[act_pid].dir_stack);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  for(int i = 0; i < 3; i++)
  {
    if(proceso[act_pid].heap[i] != m_size())
    {
      for(int a = 0; a < proceso[act_pid].heapes->actual; a++)
      {
        if(addr >= proceso[act_pid].heapes->celda[a].addr && addr <= proceso[act_pid].heapes->celda[a].addr + proceso[act_pid].heapes->celda[a].tam)
        {
          return 1;
        }
      }
    }
  }
  *out = m_read(addr);
    return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  for(int i = 0; i < 3; i++)
  {
    if(proceso[act_pid].heap[i] != m_size())
    {
      for(int a = 0; a < proceso[act_pid].heapes->actual; a++)
      {
        if(addr >= proceso[act_pid].heapes->celda[a].addr && addr <= proceso[act_pid].heapes->celda[a].addr + proceso[act_pid].heapes->celda[a].tam)
        {
          return 1;
        }
      }
    }
  }
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  if(proceso[process.pid].pid == -1)
  {
    proceso[process.pid].pid = process.pid;
    size_t size = process.program->size;
    int i = 0;
    while (size != 0)
    {
      addr_t a = reservar_memory(50);
      if(a == m_size())
      {
        exit(1);
      }
      proceso[process.pid].codigo[i] = a;
      if(size > 50)
      {
        size = size - 50;
      }
      else
      {
        size = 0;
      }
      i++;
      m_set_owner(a, a + 50);
    }
    for(int a = 0; a < 3; a++)
    {
      proceso[process.pid].heap[a] = m_size();
      proceso[process.pid].stack[a] = m_size();
    }
    proceso[process.pid].act_stack = 0;
  }
  act_pid = process.pid;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for(int i = 0; i < 7; i++)
  {
    if(proceso[process.pid].codigo[i] != m_size())
    {
      liberar_memory(proceso[process.pid].codigo[i], 50);
      m_unset_owner(proceso[process.pid].codigo[i], proceso[process.pid].codigo[i] + 50);
    }
  }
  for(int a = 0; a < 3; a++)
  {
    if(proceso[process.pid].heap[a] != m_size())
    {
      liberar_memory(proceso[process.pid].heap[a], 50);
      m_unset_owner(proceso[process.pid].heap[a], proceso[process.pid].heap[a] + 50);
    }
    if(proceso[process.pid].stack[a] != m_size())
    {
      liberar_memory(proceso[process.pid].stack[a], 50);
      m_unset_owner(proceso[process.pid].stack[a], proceso[process.pid].stack[a] + 50);
    }
  }
  proceso[process.pid].pid = -1;
}
//inicializa la free list de la memoria
void init(){
  lista = (list_t *)malloc(sizeof(list_t));
  lista->celda = (casilla_t *)malloc(100 * sizeof(casilla_t));
  lista->actual = 1;
  lista->size = 100; 
  lista->celda[0].addr = 0;
  lista->celda[0].tam = m_size() - 1;
}
//Reserva un espacio de tamaño tam en la free list
addr_t reservar_memory(size_t tam)
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
void liberar_memory(addr_t direc, size_t tam)
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
    lista->celda = (casilla_t*)realloc(lista->celda, sizeof(casilla_t) * lista->size * 2);
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
//Toma la free list del heap que esta en una pagina de un proceso y reserva un espacio dado
//si no es posible devuelve un valor igual al size de la memoria
addr_t res_memory(size_t tam, list_t l)
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
      //printf("%zd", l.celda[i].addr);
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
//Dado una free list del heap de un proceso almacenado en una pagina libera en la direccion
//especificada el espacio dado
int lib_memory(list_t l, ptr_t ptr)
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
    l.celda = (casilla_t*)realloc(lista->celda, sizeof(casilla_t) * lista->size * 2);
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
