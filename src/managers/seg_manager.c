#include "seg_manager.h"

#include "stdio.h"

#pragma region
// FreeList
#pragma region
typedef struct Free_List
{
    int m_size;
    int *data;
} Free_list_t_seg;
Free_list_t_seg *Build_Free_List_seg(int m_size)
{
    Free_list_t_seg *f = malloc(sizeof(Free_list_t_seg));
    f->m_size = m_size;
    f->data = (int *)malloc(sizeof(int) * m_size + 1);
    for (int i = 0; i < m_size; i++)
    {
        f->data[i] = 0;
    }
    return f;
}
addr_t get_last_position_seg(Free_list_t_seg *f)
{
    addr_t max = 0;
    for (int i = 0; i < f->m_size; i++)
    {
        addr_t x = f->data[i];
        max = (x == (addr_t)1) ? (addr_t)i : max;
    }
    return max;
}
addr_t find_free_space_seg(Free_list_t_seg *f, int size)
{
    // Va iterando por todas las posciones buscando una cantidad
    // contigua de espacio que sea igual al size, y toma la
    // primera que encuentre.
    int count = 0;
    for (int i = 0; i < f->m_size; i++)
    {
        count = (f->data[i]) ? 0 : count + 1;
        if (count == size)
            return i - count + 1;
    }
    return -1;
}
addr_t allocate_space_seg(Free_list_t_seg *f, int size)
{
    // Retorna el valor de la direccion donde se guardaran los
    // datos.
    addr_t adr = find_free_space_seg(f, size);
    if (adr != (addr_t)-1)
    {
        for (addr_t i = adr; i < adr + size; i++)
        {
            f->data[i] = 1;
        }
    }
    return adr;
}

addr_t free_space_seg(Free_list_t_seg *f, addr_t adr, int size)
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

typedef struct Stack
{
    addr_t addr;
    addr_t pointer;
    int size;
} Stack_t;

typedef struct Heap
{
    addr_t addr;
    addr_t size;
} Heap_t;

typedef struct Context
{
    int pid;
    int size;
    Stack_t *stack;
    Heap_t *heap;
    Free_list_t_seg *free_list;
} Context_t;

Context_t *New_Context_seg(process_t *proc, int context_size,int sub_size)
{
    Context_t *result = (Context_t *)malloc(sizeof(Context_t));
    result->pid = proc->pid;
    result->size = proc->program->size;
    result->heap = (Heap_t *)malloc(sizeof(Heap_t));
    result->stack = (Stack_t *)malloc(sizeof(Stack_t));
    result->free_list = Build_Free_List_seg(sub_size);
    return result;
}
// I need an array for the contexts
// how do i delete ?
typedef struct Context_list
{
    int count;
    Context_t **contexts;
} Context_List_t_seg;

Context_List_t_seg *New_Context_List_seg(int size)
{
    Context_List_t_seg *result = malloc(sizeof(Context_List_t_seg));
    result->count = 0;
    result->contexts = malloc(sizeof(Context_t) * size);
    return result;
}
void add_context_to_list_seg(Context_List_t_seg *contextList, Context_t *context)
{
    contextList->contexts[contextList->count] = context;
    contextList->count++;
}
int delete_context_list_seg(Context_List_t_seg *contextList, int pid)
{
    for (int i = 0; i < contextList->count; i++)
    {
        if (contextList->contexts[i]->pid == pid)
        {
            Context_t *ptr = contextList->contexts[i];
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

int context_size = 512;
int sub_context_size = 256;
Context_List_t_seg *contextList_seg;
Context_t *curr_seg;
Free_list_t_seg *freeList_seg;

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
    contextList_seg = New_Context_List_seg(m_size());
    freeList_seg = Build_Free_List_seg(m_size());
    return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
    // printf("##pid %d \n", curr_seg->pid);
    // printf("##size %d \n", curr_seg->size);
    // printf("##heap pointer %lld \n", curr_seg->heap->addr);
    // printf("##stack pointer %lld \n", curr_seg->stack->addr);

    addr_t adr = allocate_space_seg(curr_seg->free_list, size);
    addr_t m_adr = curr_seg->heap->addr + adr; 
    out->addr = m_adr;
    out->size = size;
    return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
    free_space_seg(curr_seg->free_list,ptr.addr,ptr.size);
    return 0;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
    addr_t m_adr = curr_seg->stack->addr + curr_seg->stack->pointer;
    m_write(m_adr,val);
    curr_seg->stack->pointer++;
    out->addr = m_adr;
    out->size = 1;
    return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
    curr_seg->stack->pointer--;
    addr_t m_adr = curr_seg->stack->addr + curr_seg->stack->pointer;
    *out = m_read(m_adr);
    return 0;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
  m_write(addr,val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
    for (int i = 0; i < contextList_seg->count; i++)
    {
        if (contextList_seg->contexts[i]->pid == process.pid)
        {
            curr_seg = contextList_seg->contexts[i];
            return;
        }
    }
    // Crear
    // Allocar
    // Guardar
    Context_t *new = New_Context_seg(&process, context_size,sub_context_size);
    addr_t heap_adr = allocate_space_seg(freeList_seg, sub_context_size);
    new->heap->addr = heap_adr;
    new->heap->size = sub_context_size;
    m_set_owner(new->heap->addr, new->heap->addr + new->heap->size);

    addr_t stack_adr = allocate_space_seg(freeList_seg, sub_context_size);
    new->stack->addr = stack_adr;
    new->stack->size = sub_context_size;
    new->stack->pointer = 0;
    m_set_owner(new->stack->addr, new->stack->addr + new->stack->size);
    
    add_context_to_list_seg(contextList_seg, new);
    curr_seg = new;
    return;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
    
    printf("##pid %d \n", curr_seg->pid);
    printf("##size %d \n", curr_seg->size);
    printf("##heap pointer %lld \n", curr_seg->heap->addr);
    printf("##stack pointer %lld \n", curr_seg->stack->addr);

    m_unset_owner(curr_seg->heap->addr  ,curr_seg->heap->addr  + curr_seg->heap->size);
    m_unset_owner(curr_seg->stack->addr ,curr_seg->stack->addr + curr_seg->stack->size);
    
    free_space_seg(freeList_seg,curr_seg->heap->addr,curr_seg->heap->size);
    free_space_seg(freeList_seg,curr_seg->stack->addr,curr_seg->stack->size);
    delete_context_list_seg(contextList_seg,process.pid);
}
