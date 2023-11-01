#include "pag_manager.h"

#include "stdio.h"

#pragma region 
#pragma region 
typedef struct Free_List
{
    int m_size;
    int* data; 
} Free_list_t;
Free_list_t* Build_Free_List(int m_size)
{
    Free_list_t* f = malloc(sizeof(Free_list_t));
    f->m_size = m_size;  
    f->data = (int*) malloc(sizeof(int)*m_size + 1);
    for (int i = 0; i < m_size; i++)
    {
        f->data[i] = 0;
    }
    return f;
}
addr_t get_last_position(Free_list_t* f)
{
    addr_t max = 0;
    for (addr_t i = 0; i < f->m_size; i++)
    {
        addr_t x = f->data[i];
        max = (x == 1)? i : max;  
    }
    return max;
}
addr_t find_free_space(Free_list_t* f, int size)
{
    // Va iterando por todas las posciones buscando una cantidad 
    // contigua de espacio que sea igual al size, y toma la 
    // primera que encuentre. 
    addr_t count = 0; 
    for (addr_t i = 0; i < f->m_size; i++)
    {
        count = (f->data[i])? 0 : count+1;
        if (count == size)
            return i-count+1;
    }
    return -1;
}
addr_t allocate_space(Free_list_t* f, int size)
{
    // Retorna el valor de la direccion donde se guardaran los 
    // datos.
    addr_t adr = find_free_space(f,size);
    if(adr != -1)
    {
        for (addr_t i = adr; i < adr+size; i++)
        {
            f->data[i] = 1;
        }
    }
    return adr; 
}

addr_t free_space(Free_list_t* f,addr_t adr, int size)
{
    // Retorna el valor de la direccion donde se guardaran los 
    // datos.
    for (addr_t i = adr; i < adr+size; i++)
    {
        if(f->data[i]== 0)
            return -1;
    }
    for (addr_t i = adr; i < adr+size; i++)
    {
        f->data[i] = 0;
    }
    return 1;
}
#pragma endregion

typedef struct Context
{
    int pid; 
    int size; 
    int heap_pages_count;
    int stack_pages_count;
    addr_t stack_pointer; 
    int* heap_pages;   
    int* stack_pages;   
    Free_list_t *free_list;
} Context_t;

Context_t* New_Context(process_t *proc,int context_size, int pfn)
{
    Context_t* result = (Context_t*)malloc(sizeof(Context_t));
    result->pid = proc->pid;
    result->size = proc->program->size;
    result->heap_pages_count = 1;
    result->stack_pages_count = 0;
    result->stack_pointer = 0;
    result->heap_pages = (int*)malloc(context_size);
    result->stack_pages = (int*)malloc(context_size);
    result->free_list = Build_Free_List(context_size);
    // Putting the size . 
    allocate_space(result->free_list,result->size);
    result->heap_pages[0] = pfn;
    return result;
}
// I need an array for the contexts 
// how do i delete ?
typedef struct Context_list
{
    int count;
    Context_t** contexts;
}Context_List_t;

Context_List_t* New_Context_List(int size)
{
    Context_List_t* result = malloc(sizeof(Context_List_t));
    result->count = 0;
    result->contexts = malloc(sizeof(Context_t)*size);
    return result;
}
void add_context_to_list(Context_List_t* contextList, Context_t* context)
{
    contextList->contexts[contextList->count] = context;
    contextList->count ++ ;
}
delete_context_list(Context_List_t* contextList, int pid)
{
    for (size_t i = 0; i < contextList->count; i++)
    {
        if(contextList->contexts[i]->pid == pid)
        {
            Context_t* ptr = contextList->contexts[i];
            for (size_t j = i; j < contextList->count-1; j++)
            {
                contextList->contexts[j] = contextList->contexts[j+1];
                free(ptr);
                contextList->count--;
                return 0;
            }
        }
    }
    return 1;
}

addr_t translate_vpf(int page_size,int page, addr_t v_adr)
{
    return page_size*page+v_adr;
}
#pragma endregion


int context_size_pag = 1024;
int pag_size = 256;
Context_List_t* contextList_pag;
Context_t* curr_pag; 
Free_list_t* freeList_pag;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  contextList_pag = New_Context_List(context_size_pag);
  freeList_pag = Build_Free_List(m_size()/pag_size);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  addr_t adr = allocate_space(curr_pag->free_list, size);
  int page_index = adr/pag_size;
  if(page_index > curr_pag->heap_pages_count)
  {
    return 1;
  }
  int pfn = curr_pag->heap_pages[page_index];
  addr_t offset = adr%pag_size;
  out->addr = pfn*pag_size + offset;
  out->size = size; 
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  // Ver si tengo pagina para stack
  if(curr_pag->stack_pages_count == 0)
  {
    addr_t adr = allocate_space(freeList_pag,1);
    curr_pag->stack_pages[0] = adr;
    curr_pag->stack_pages_count++;
    m_set_owner(adr*pag_size,(adr+1)*pag_size);
    printf("!!adr %d \n", adr);
  }
  // aqui ya tengo stack . 
  addr_t m_adr = curr_pag->stack_pointer + pag_size*curr_pag->stack_pages[curr_pag->stack_pages_count-1];
  printf("!!m_adr %d \n", m_adr);
  curr_pag->stack_pointer++;
  m_write(m_adr,val);
  out->addr = m_adr;
  out->size = 1;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  curr_pag->stack_pointer--;
  int pfa = curr_pag->stack_pages[curr_pag->stack_pages_count-1];
  addr_t m_adr = pfa*pag_size + curr_pag->stack_pointer;
  *out = m_read(m_adr);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  m_write(addr,val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  for (size_t i = 0; i < contextList_pag->count; i++)
  {
    if(contextList_pag->contexts[i]->pid == process.pid)
    {
      curr_pag = contextList_pag->contexts[i];
      return;
    }
  }
  // Crear proceso nuevo
  // Allocarlo
  // Persistirlo 
  addr_t adr = allocate_space(freeList_pag,1);
  Context_t* con = New_Context(&process,context_size_pag,adr);
  m_set_owner(pag_size*adr,pag_size*(adr+1));
  add_context_to_list(contextList_pag,con);
  curr_pag = con;
  return;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for (size_t i = 0; i < curr_pag->heap_pages_count; i++)
  {
    int t = curr_pag->heap_pages[i];
    freeList_pag->data[t] = 0;
    m_unset_owner(pag_size*t,pag_size*(t+1));
  }
  for (size_t i = 0; i < curr_pag->stack_pages_count; i++)
  {
    int t = curr_pag->stack_pages[i];
    freeList_pag->data[t] = 0;
    m_unset_owner(pag_size*t,pag_size*(t+1));
  }
  delete_context_list(contextList_pag,process.pid);
}
