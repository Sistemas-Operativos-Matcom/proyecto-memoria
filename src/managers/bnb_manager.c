#include "bnb_manager.h"
#include "stdio.h"


// La intencion de poner todo este codigo aqui 
// fue que me perimitiera usar el debuger, por alguna razon
// cuando importaba el codigo el debuger crasheaba. 

// Structures
#pragma region
// FreeList
#pragma region

typedef struct Free_List_bnb
{
  addr_t m_size;
  int *data;
} Free_list_t_bnb;
Free_list_t_bnb *Build_Free_List_bnb(addr_t m_size)
{
  Free_list_t_bnb *f = malloc(sizeof(Free_list_t_bnb));
  f->m_size = m_size;
  f->data = (int *)malloc(sizeof(int) * m_size + 1);
  for (addr_t i = 0; i < m_size; i++)
  {
    f->data[i] = 0;
  }
  return f;
}
addr_t get_last_position_bnb(Free_list_t_bnb *f)
{
  addr_t max = 0;
  for (addr_t i = 0; i < f->m_size; i++)
  {
    addr_t x = f->data[i];
    max = (x == 1) ? i : max;
  }
  return max;
}
addr_t find_free_space_bnb(Free_list_t_bnb *f, addr_t size)
{
  // Va iterando por todas las posciones buscando una cantidad
  // contigua de espacio que sea igual al size, y toma la
  // primera que encuentre.
  addr_t count = 0;
  for (addr_t i = 0; i < f->m_size; i++)
  {
    count = (f->data[i]) ? 0 : count + 1;
    if (count == size)
      return i - count + 1;
  }
  return -1;
}
addr_t allocate_space_bnb(Free_list_t_bnb *f, addr_t size)
{
  // Retorna el valor de la direccion donde se guardaran los
  // datos.
  addr_t adr = find_free_space_bnb(f, size);
  if (adr != (addr_t)-1)
  {
    for (addr_t i = adr; i < adr + size; i++)
    {
      f->data[i] = 1;
    }
  }
  return adr;
}

addr_t free_space_bnb(Free_list_t_bnb *f, addr_t adr, addr_t size)
{
  // Retorna el valor de la direccion donde se guardaran los
  // datos.
  for (addr_t i = adr; i < adr + size; i++)
  {
    if (f->data[i] == 0)
      return -1;
  }
  for (addr_t i = adr; i < adr + size; i++)
  {
    f->data[i] = 0;
  }
  return 1;
}

#pragma endregion
// Context
#pragma region
typedef struct Context
{
  int pid;
  int size;
  addr_t base;
  addr_t bound;
  addr_t heap_pointer;
  addr_t stack_pointer;
  Free_list_t_bnb *free_list;
} Context_t_bnb;

Context_t_bnb *New_Context_bnb(process_t *proc, int context_size, addr_t adr)
{
  Context_t_bnb *result = (Context_t_bnb *)malloc(sizeof(Context_t_bnb));
  result->pid = proc->pid;
  result->size = proc->program->size;
  result->base = adr;
  result->bound = adr + context_size;
  result->heap_pointer = adr + result->size;
  result->stack_pointer = result->bound-1;
  result->free_list = Build_Free_List_bnb(context_size);
  allocate_space_bnb(result->free_list,result->size);
  return result;
}
#pragma endregion
// Context List
#pragma region
typedef struct Context_list
{
  int count;
  Context_t_bnb **contexts;
} Context_List_t_bnb;

Context_List_t_bnb *New_Context_List_bnb(int size)
{
  Context_List_t_bnb *result = malloc(sizeof(Context_List_t_bnb));
  result->count = 0;
  result->contexts = malloc(sizeof(Context_t_bnb) * size);
  return result;
}
void add_context_to_list_bnb(Context_List_t_bnb *contextList, Context_t_bnb *context)
{
  contextList->contexts[contextList->count] = context;
  contextList->count++;
}
addr_t delete_context_list_bnb(Context_List_t_bnb *contextList, int pid)
{
  for (int i = 0; i < contextList->count; i++)
  {
    if (contextList->contexts[i]->pid == pid)
    {
      Context_t_bnb *ptr = contextList->contexts[i];
      for (int j = i; j < contextList->count - 1; j++)
      {
        contextList->contexts[j] = contextList->contexts[j + 1];
        free(ptr);
        contextList->count--;
        return 0;
      }
    }
  }
  return 1;
}
#pragma endregion
#pragma endregion

int context_size_bnb = 1000;
Context_List_t_bnb *contextList;
Free_list_t_bnb *freeList;
Context_t_bnb* curr; 

// printf("##pid %lld \n", curr->pid);
// printf("##base %lld \n", curr->base);
// printf("##bound %lld \n", curr->bound);
// printf("##size %lld \n", curr->size);
// printf("##heap pointer %lld \n", curr->heap_pointer);
// printf("##stack pointer %lld \n", curr->stack_pointer);

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  printf("!!Init\n");
  contextList = New_Context_List_bnb(m_size());
  freeList = Build_Free_List_bnb(m_size());
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{

  printf("!!Malloc\n");
  addr_t adr = allocate_space_bnb(curr->free_list, size);
  if(adr == (addr_t)-1)
    return 1;
  // Tenemos el address
  curr->heap_pointer = get_last_position_bnb(curr->free_list);
  printf("##adr fake%lld \n", adr);
  printf("##adr real and returned %lld \n", curr->base+adr);

  out->addr = curr->base+adr;
  out->size = size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  // puede que este bugeado... :)
    for (size_t i = ptr.addr; i < ptr.addr+ptr.size; i++)
    {
      curr->free_list->data[i] = 0;
    }
    return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  addr_t m_adr = curr->stack_pointer;
  // printf("##m_adr %lld \n", m_adr);
  m_write(m_adr,val);
  out->addr = m_adr;
  out->size = 1;
  curr->stack_pointer--;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  curr->stack_pointer++; 
  *out = m_read(curr->stack_pointer);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if(curr->bound<=addr)
    return 1;
  m_write(addr,val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  // To make the context switch as they are supposed to be done
  // I need to have some persistence of the data of the current proccess
  // that I have in memory.
  printf("!!Context Switch\n");
  for (int i = 0; i < contextList->count; i++)
  {
    if(contextList->contexts[i]->pid == process.pid)
    {
      // El proceso ya esta en la memoria 
      curr = contextList->contexts[i];
      return; 
    }
  }
  // El proceso no esta en memoria 

  // Crear proceso 
  // Guardarlo en nuestra persistencia de datos
  // Ponerlo en memoria 
  // Actualizar el curr.
  addr_t adr = allocate_space_bnb(freeList,context_size_bnb);
  if(adr == (addr_t)-1)
  {
    fprintf(stderr, "No hay espacio en memoria para el proceso : ContextSwitch");
    exit(1);
  }
  Context_t_bnb* new = New_Context_bnb(&process, context_size_bnb,adr);
  add_context_to_list_bnb(contextList,new);
  m_set_owner(new->base,new->bound);
  curr = new;
  return;
}
// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  for (int i = 0; i < contextList->count; i++)
  {
    if(contextList->contexts[i]->pid == process.pid)
    {
      m_unset_owner(contextList->contexts[i]->base,contextList->contexts[i]->bound);
      break;
    }
  }
  delete_context_list_bnb(contextList,process.pid);
}
