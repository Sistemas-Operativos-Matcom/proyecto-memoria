#include "bnb_manager.h"
#include "../memory.h"

#include "stdio.h"

//Estructura que representa un espacio reservado en el heap
typedef struct Reserved_Space
{
  size_t initSpace;
  size_t endSpace;
  struct Reserved_Space *next;
  struct Reserved_Space *previous;
} Reserved_Space;

//Estructura que representa a un proceso
typedef struct process_bnb {
  int pid;
  size_t base;
  size_t heap_pointer;
  size_t stack_pointer;
  Reserved_Space *root; 
  Reserved_Space *final; 
}process_bnb;

//Variables globales
size_t curr_process ;// Proceso actual
const size_t bounds_process = 512;//Tamaño del bloque
size_t cant_process; //Cantidad de bloques
process_bnb *process_arr; //Array que contiene todos los bloques == Memoria virtual

//Metodos para la lista enlazada
//.Inserta un nuevo espacio reservado
void Insert(size_t proc, size_t initSpace, size_t endSpace)
{
  //Crea el nuevo espacio e inicializa sus valores
  Reserved_Space *new = (Reserved_Space*)malloc(sizeof(Reserved_Space));
  new -> initSpace = initSpace;
  new -> endSpace = endSpace;
  new->next = NULL;
  new->previous = process_arr[proc].final;

  if (process_arr[proc].root == NULL)//Si la lista esta vacia
  {
    process_arr[proc].root = new; //Se iguala la raiz y el final de la lista al nodo creado
    process_arr[proc].final = new;
  }
  else //Si la lista tiene elementos se inserta el nodo al final de la lista
  {
    (process_arr[proc].final)->next = new;
    process_arr[proc].final = new;
  }
}

//Eliminar un espacio reservado
int Delete(size_t proc, size_t addrToDelete)
{
  Reserved_Space *curr_space = process_arr[proc].root; //Inicializando el espacio actual como la raiz
  
  while (curr_space != NULL) 
  {
    if((curr_space -> initSpace) == addrToDelete) //Si el espacio reservado es igual al addr
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

  if (curr_space == process_arr[proc].root) // Si el proceso es el principio de la lista
  {
    if (process_arr[proc].root == process_arr[proc].final) // Un solo elemento en la cola
    {
      process_arr[proc].root = NULL;
      process_arr[proc].final = NULL;
    }
    else // Mas de un elemento en la cola
    {
      (curr_space->next)->previous = NULL;
      process_arr[proc].root = (process_arr[proc].root)->next;
    }
  }
  else if (curr_space == process_arr[proc].final) // Si el proceso es el final de la cola
  {
    (curr_space->previous)->next = NULL;
    process_arr[proc].final = (process_arr[proc].final)->previous;
  }
  else // Si el proceso esta en el medio de la cola
  {
    (curr_space->next)->previous = curr_space->previous;
    (curr_space->previous)->next = curr_space->next;
  }

  free(curr_space);
  return 1;
}

int Search(size_t proc, size_t addrToSearch)
{
  Reserved_Space *aux = process_arr[proc].root;
  while(aux != NULL)
  {
    if(addrToSearch >= aux->initSpace && addrToSearch <= aux->endSpace)
    {
      return 1;
    }

    aux = aux->next;
  }

  return 0;
}


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) { //OK
  cant_process = m_size()/bounds_process; //Hallando la cantidad total de procesos
  process_arr = (process_bnb*)malloc(sizeof(process_bnb)*cant_process); //Reservando el espacio de toda la memoria  
  curr_process = -1; //Inicializando el pid del proceso actual, como al principio no hay ninguno entonces -1

  for(size_t i=0, init = 0; i<cant_process; i++, init+=bounds_process) //Iterando por cada uno de los bloques y asignandoles el espacio que le corresponde a cada uno
  {
    process_arr[i].pid = NO_ONWER;
    process_arr[i].base = init;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  if((process_arr[curr_process].stack_pointer) - (process_arr[curr_process].heap_pointer) >= size) //Si el espacio libre es mayor o igual que el size
  {
    out->addr = process_arr[curr_process].heap_pointer; //Actualizando los valores de la variable out
    out->size = size;

    process_arr[curr_process].heap_pointer += size; //Actualizando el valor del heap_pointer
    Insert(curr_process, out->addr, out->addr + out->size - 1); //Inserta el nuevo espacio reservado

    return 0;
  }

  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  if((ptr.addr < process_arr[curr_process].heap_pointer) && Delete(curr_process, ptr.addr))
  {
    if(ptr.addr+ptr.size == process_arr[curr_process].heap_pointer)
    {
      process_arr[curr_process].heap_pointer -= ptr.size;  
    }
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {//OK
  size_t endH = process_arr[curr_process].heap_pointer;//Final del heap
  size_t initS = process_arr[curr_process].stack_pointer;//Final del stack

  if((initS-endH)>=1)//Si hay al menos un byte libre entre el heap y el stack
  {
    //Actualizando el puntero out
    out->addr = process_arr[curr_process].stack_pointer;
    out->size = 1;

    //Escribiendo el valor en el stack
    m_write(process_arr[curr_process].stack_pointer + process_arr[curr_process].base, val);
    
    (process_arr[curr_process].stack_pointer)--;//Actualizando el puntero del stack

    return 0;
  }

  return 1;
}

// Quita un elemento del stack        
int m_bnb_pop(byte *out) {//OK
  //Si el stack_pointer se encuentra al final del bloque o después de él
  if(process_arr[curr_process].stack_pointer >= bounds_process-1) 
  {                                                         
    return 1;
  }

  // Almacenando en la variable out el valor al que le hice pop
  *out = m_read(process_arr[curr_process].stack_pointer+1 + process_arr[curr_process].base); 
  process_arr[curr_process].stack_pointer ++; //Disminuyendo el tamaño del stack (que se traduce en aumentar su dirección en memoria)
  return 0;
}

// Carga el valor en una dirección determinada   
int m_bnb_load(addr_t addr, byte *out) {//OK
  //Si addr es mayor que el tamaño asignado para un proceso
  if(addr >= bounds_process)
  {
    return 1;
  }

  *out = m_read(addr + process_arr[curr_process].base); //Almacenando en la variable out el valor que aparece en la direccion addr
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {//OK
  //Si addr está dentro del heap y además se encuentra entre los espacios reservados
  if(addr < process_arr[curr_process].heap_pointer && Search(curr_process, addr))
  {
    m_write(addr + process_arr[curr_process].base, val);//Escribiendo dicho valor en el heap
    return 0;
  }

  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {//OK
  int empty_space;//Primer bloque vacío
  int band = 0;//Si está en 1 entonces se encontró un bloque vacío, si está en cero entonces en todos los bloques hay un proceso
  
  for(size_t i=0; i<cant_process; i++)
  {
    if((process_arr[i].pid == -1) && band==0) //Buscando el primer bloque de la memoria que esté vacío
    {
      empty_space = i;
      band = 1;
    }
    else if(process.pid == process_arr[i].pid) //Si se encontró al proceso, actualizar el curr_process
    {
      curr_process = i; 
      return;
    }
  }

  if(band==1) //Si hay un espacio vacio, guardamos el proceso en dicho lugar
  {
    process_arr[empty_space].pid = process.pid; //Actualizando las propiedades del proceso nuevo
    process_arr[empty_space].heap_pointer = 0;
    process_arr[empty_space].stack_pointer = bounds_process-1;
    process_arr[empty_space].root = NULL;
    process_arr[empty_space].final = NULL;
    curr_process = empty_space;
    m_set_owner(process_arr[empty_space].base, process_arr[empty_space].base + bounds_process);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  // Buscamos el pid en los slices y se asigna a -1
  for (size_t i = 0; i < cant_process; i++) {
    if (process_arr[i].pid == process.pid) {
      m_unset_owner(process_arr[i].base, process_arr[i].base + bounds_process);
      process_arr[i].pid = -1;
      return;
    }
  }
}