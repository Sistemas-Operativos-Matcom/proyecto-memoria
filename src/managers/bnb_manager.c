#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"

//Estructura que representa un espacio reservado en el heap
typedef struct Reserved_Space
{
  size_t initH;
  size_t endH;
  struct Reserved_Space *next;
  struct Reserved_Space *previous;
} Reserved_Space;

//Estructura que representa a un proceso
typedef struct process_bnb {
  int pid;
  size_t size;
  size_t base;
  size_t start_heap;
  size_t heap_pointer;
  size_t stack_pointer;
  size_t end_process;
  Reserved_Space *root; 
  Reserved_Space *final; 
}process_bnb;

//Variables globales
process_bnb* curr_process ;// Proceso actual
size_t bounds_process = 1024;//Tamaño del bloque
size_t cant_process; //Cantidad de bloques
process_bnb *process_arr; //Array que contiene todos los bloques == Memoria virtual

//Metodos para la lista enlazada
//.Inserta un proceso a la cola especificada
void Insert(process_bnb* proc, size_t initSpace, size_t endSpace)
{
  Reserved_Space *new = (Reserved_Space*)malloc(sizeof(Reserved_Space));
  new -> initH = initSpace;
  new -> endH = endSpace;
  new->next = NULL;
  new->previous = proc->final;

  if (proc->root == NULL)//Si la lista esta vacia
  {
    proc->root = new; //Se iguala la raiz y el final de la lista al nodo creado
    proc->final = new;
  }
  else //Si la lista tiene elementos se inserta el nodo al final de la lista
  {
    (proc->final)->next = new;
    proc->final = new;
  }
}

//Eliminar un espacio reservado
int Delete(process_bnb* proc, size_t addrToDelete)
{
  Reserved_Space *curr_space = proc->root; //Inicializando el espacio actual como la raiz
  
  while (curr_space != NULL) 
  {
    if((curr_space -> initH) == addrToDelete) //Si el espacio reservado es igual al addr
    {
      break;
    }
    curr_space = curr_space->next;
  }

  // Si el espacio es NULL
  if (curr_space == NULL)
  {
    return 0;
  }

  if (curr_space == proc->root) // Si el proceso es el principio de la lista
  {
    if (proc->root == proc->final) // Un solo elemento en la cola
    {
      proc->root = NULL;
      proc->final = NULL;
    }
    else // Mas de un elemento en la cola
    {
      (curr_space->next)->previous = NULL;
      proc->root = (proc->root)->next;
    }
  }
  else if (curr_space == proc->final) // Si el proceso es el final de la cola
  {
    (curr_space->previous)->next = NULL;
    proc->final = (proc->final)->previous;
  }
  else // Si el proceso esta en el medio de la cola
  {
    (curr_space->next)->previous = curr_space->previous;
    (curr_space->previous)->next = curr_space->next;
  }

  free(curr_space);
  return 1;
}


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) { //OK
  cant_process = m_size()/bounds_process;
  process_arr = (process_bnb*)malloc(sizeof(process_bnb)*cant_process); //Reservando el espacio de toda la memoria
  for(int i=0; i<cant_process; i++) //Reservando espacio para cada una de las estructuras
  {
    process_bnb *proc = (process_bnb*) malloc(sizeof(process_bnb));
    process_arr[i] = *proc;
    (process_arr[i]).root = NULL;//Inicializando en NULL la raiz de la lista
    (process_arr[i]).final = NULL;//Inicializando en NULL el final de la lista
  }
  
  curr_process = (process_bnb*)malloc(sizeof(process_bnb)); //Reservando espacio el proceso actual

  process_bnb aux;
  for(int i=0, init = 0; i<cant_process; i++, init+=bounds_process) //Iterando por cada uno de los bloques y asignandoles el espacio que le corresponde a cada uno
  {
    aux = process_arr[i]; 
    aux.pid = NO_ONWER;
    aux.base = init;
    aux.stack_pointer = init+bounds_process-1;
    aux.end_process = init+bounds_process-1;
    m_set_owner(init, init+bounds_process);//Asignandole pid=-1 a todas las direcciones de la memoria 
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {//OK
  if((curr_process->stack_pointer) - (curr_process->heap_pointer) >= size) //Si el espacio libre es mayor o igual que el size
  {
    out->addr = curr_process->heap_pointer;
    curr_process->heap_pointer += size; //Actualizar valor del heap_pointer
    out->size = size;
    m_set_owner(out->addr, curr_process->heap_pointer); //Asignandole el pid del proceso al espacio reservado
    Insert(curr_process, out->addr, (curr_process->heap_pointer)-1);

    return 0;
  }

  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {//Falta arreglar lo de la lista de espcaios reservados en este metodo y en el malloc
  size_t initH = curr_process->start_heap;
  size_t endH = (curr_process->heap_pointer)-1;
  size_t curr_addr = (curr_process->base)+ptr.addr;

  if((curr_addr >= initH) && ((curr_addr <= endH)) && Delete(curr_process, curr_addr))
  {
    m_unset_owner(curr_addr, curr_addr+ptr.size);
    if(curr_addr+ptr.size-1 == endH)
    {
      (curr_process->heap_pointer) -= ptr.size;  
    }
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {//OK
  size_t endH = curr_process->heap_pointer;//Final del heap
  size_t initS = curr_process->stack_pointer + 1;//Final del stack

  if((initS-endH)>=1)//Si hay al menos un byte libre entre el heap y el stack
  {
    out->addr = curr_process->stack_pointer;//Asignandole el addr al puntero out
    m_write(out->addr, val);//Escribiendo el valor en el stack
    m_set_owner(out->addr, (out->addr)+1);//Asignandole el pid del proceso actual a la posicion del stack
    out->size = 1;
    (curr_process->stack_pointer)--;//Actualizando el puntero del stack

    return 0;
  }

  return 1;
}

// Quita un elemento del stack        
int m_bnb_pop(byte *out) {//OK
  if((curr_process->stack_pointer) == (curr_process->end_process)) //Si el puntero que apunta al stack se encuentra al final del bloque, entonces
  {                                                         // significa que el stack esta vacio y no se ha hecho push a ningun valor
    return 1;
  }

  *out = m_read((curr_process->stack_pointer)-1); // Almacenando en la variable out el valor al que le hice pop
  m_unset_owner((curr_process->stack_pointer)-1, (curr_process->stack_pointer));
  (curr_process->stack_pointer)++; //Disminuyendo el tamaño del stack (que se traduce en aumentar su dirección en memoria)
  return 0;
}

// Carga el valor en una dirección determinada   
int m_bnb_load(addr_t addr, byte *out) {//OK
  size_t initH = curr_process->start_heap;
  size_t endH = (curr_process->heap_pointer)-1;
  size_t initS = (curr_process->stack_pointer)-1;
  size_t endS = curr_process->end_process;
  size_t curr_addr = (curr_process->base)+addr;
  
  if((curr_addr >= initH && curr_addr <= endH) || (curr_addr >= initS && curr_addr <= endS)) //Si 'curr_addr' está en el heap o en el stack 
  {
    *out = m_read(curr_addr); //Almacenando en la variable out el valor que aparece en la direccion addr
    return 0;
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {//OK
  size_t initH = curr_process->start_heap;
  size_t endH = (curr_process->heap_pointer)-1;
  size_t curr_addr = (curr_process->base)+addr;//Valor del addr en la memoria fisica

  if(curr_addr >= initH && curr_addr <= endH) //Si 'curr_addr' esta dentro del heap
  {
    m_write(curr_addr, val);//Escribiendo dicho valor en el heap
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {//OK
  int empty_space;//primer bloque vacío
  int band = 0;//Si está en 1 entonces se encontró un bloque vacío, si está en cero entonces en todos los bloques hay un proceso
  
  process_bnb *aux;
  for(int i=0; i<cant_process; i++)
  {
    aux = &process_arr[i];
    if((aux->pid == NO_ONWER) && band==0) //Buscando el primer bloque de la memoria que esté vacío
    {
      empty_space = i;
      band = 1;
    }
    else
    {
      if(process.pid == aux->pid) //Si se encontró al proceso, actualizar el curr_process
      {
        curr_process = aux; //OJO
        return;
      }
    }
  }

  if(band==1) //Si hay un espacio vacio, guardamos el proceso en dicho lugar
  {
    process_arr[empty_space].pid = process.pid; //Actualizando las propiedades del proceso nuevo
    process_arr[empty_space].size = (process.program)->size;
    process_arr[empty_space].start_heap = process_arr[empty_space].base + process_arr[empty_space].size;
    process_arr[empty_space].heap_pointer = process_arr[empty_space].base + process_arr[empty_space].size;
    curr_process = &process_arr[empty_space];
    m_set_owner(process_arr[empty_space].base, (process_arr[empty_space].start_heap));
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {//OK
  process_bnb *aux;
  
  for (int i = 0; i < cant_process; i++) {
    aux = &process_arr[i];
    if (aux->pid == process.pid) {
      m_unset_owner(aux->base, (aux->end_process)+1);
      aux->pid = -1;
      return;
    }
  }
}
