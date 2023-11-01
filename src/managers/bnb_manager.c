#include "bnb_manager.h"
#include "./stdio.h"
#include "stdlib.h"


// Estructura para representar un espacio del heap
typedef struct free_space {
    int base;
    int bound;
    struct free_space* next;
} free_space;

// Función para crear un nuevo espacio
free_space* createfree_space(int base, int bound) {
    free_space* newfree_space = (free_space*)malloc(sizeof(free_space));
    newfree_space->base = base;
    newfree_space->bound = bound;
    newfree_space->next = NULL;
    return newfree_space;
}

// Función para agrupar los espacios vacios adyacentes
void zip(free_space* temp) {
  while (temp->next != NULL)
  {
    // Agrupar el espacio libre actual al siguiente, y eliminar este
    if (temp->base + temp->bound == temp->next->base)
    {
      temp->bound = temp->bound + temp->next->bound;
      temp->next = temp->next->next;
    }

    // Si en este punto solo tengo un nodo no tengo con que comparar
    if (temp->next == NULL)
    {
      break;
    }
    
    // Si ya no puedo seguir agrupando con este nodo paso a analizar el siguiente
    if (temp->base + temp->bound != temp->next->base)
    {
        temp = temp->next;   
    }
  }
}

// Reservar espacio en el heap y retorna -1 o la dirección reservada
int reserve(free_space* heap, int bound) {
  
  // No hay espacios libres en el heap
  if (heap->base == -2) {
      return -1;
  }

  // Buscar el lugar para ocupar
  while (heap->base != -2)
  {
    heap = heap->next;
  
    if(heap->bound > bound)
    {
      heap->bound -= bound;
      return heap->bound;
    }
  }

  return -1;
}

// Función de prueba para imprimir los elementos de la lista
void printList(free_space* heap) {
  if (heap == NULL) {
      printf("La lista está vacía.\n");
      return;
  }
  
  printf("Elementos en la lista: ");
  while (heap != NULL) {
      printf(": %d - ", heap->base);
      printf("%d :", heap->bound);
      heap = heap->next;
  }
  printf("\n");
}

int belong(free_space* heap, int addr) {

  if (heap == NULL) {
    // printf("La lista está vacía.\n");
    return 0;
  }

  while (heap != NULL)
  {
    if (heap->base < addr && addr < heap->base + heap->bound)
    {
      return 1;
    }
    
    heap = heap->next;
  }

  return 0;
  
}

int isFree(free_space* heap, int base, int bound) {

  if (heap == NULL) {
    // printf("La lista está vacía.\n");
    return 0;
  }

  while (heap != NULL)
  {
    if((heap->base < base && base < heap->base+heap->bound) || (heap->base < base+bound && base+bound < heap->base+heap->bound))
    {
      return 1;
    }

    heap = heap->next;
  }

  return 0;
}

// Función para agregar un espacio en el heap
int insert(free_space* heap, int base, int bound) {
  free_space* newfree_space = createfree_space(base, bound);
  
  if (heap == NULL) {
    heap = newfree_space;
  }
  else
  {
    if (isFree(heap, base, bound))
    {
      return 1;
    }

    // Iterar por los espacios libres hasta encontrar el lugar en el que debería ir este espacio nuevo
    while (heap->next != NULL && heap->next->base < base)
    {
        heap = heap->next;
    }
    
    // Insertar el espacio libre
    newfree_space->next = heap->next;
    heap->next = newfree_space;

    // Agrupar espacios libres adyacentes
    zip(heap);
      
  }

  return 0;
}

void destroid(free_space* heap) {
  free_space* temp;
  while (heap != NULL)
  {
    temp = heap;
    heap = heap->next;
    free(temp);
  }
  free(heap);
}


// Estructura del contexto
typedef struct Context {
  int pid;
  int base;
  int heap_addr;
  free_space* heap;
  int heap_size;
  int ptr_stack;
} t_context;




// Variables globales
t_context* mem_partition; 
int current_process_index;
int total_count_process;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  current_process_index = -1;
  // Reservar espacios para trabajar con tantos procesos en la memoria segun su capacidad
  int memory = m_size(); // memoria total
  total_count_process = memory/1024;

  mem_partition = (t_context*)malloc(sizeof(t_context)*total_count_process);

  for (int i = 0; i < total_count_process; i++)
  {
    mem_partition[i].pid = -1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {

  // Reservar espacio en el heap del proceso
  int addr = reserve(mem_partition[current_process_index].heap, size);
  // FILE* fichero;
  // fichero = fopen("hola.txt", "a");
  // fprintf(fichero, "hola %d\n", 0);
  // fclose(fichero);
  if (addr == -1)
  {
    // Si en el heap no hay espacio libre comprobar que al incrementar el tamaño del heap no se solapa con el stack
    int p_stack = mem_partition[current_process_index].ptr_stack;
    if(mem_partition[current_process_index].ptr_stack == 0)
    {
      p_stack = 1024;
    }
    
    if (mem_partition[current_process_index].heap_size + (int)size >= p_stack)
    {
      // printf("%d\n",(int)size);
      // printf("%d\n",mem_partition[current_process_index].ptr_stack);
      printf("No se puede reservar espacio porque no hay suficiente\nProgram Terminated\n");
		  return 1;
    }
    else
    {
      out->addr = mem_partition[current_process_index].heap_size;
      mem_partition[current_process_index].heap_size += size;
      return 0;
    }
  }

  out->addr = addr;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
// (preguntar si liberar un espacio que ya estaba libre debe ser causa de algun error o retornar 1 ===== si)
int m_bnb_free(ptr_t ptr) {

  return insert(mem_partition[current_process_index].heap ,ptr.addr ,ptr.size);
  
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  
  m_write(mem_partition[current_process_index].ptr_stack, val);
  out = m_read(mem_partition[current_process_index].ptr_stack);
  // printf("direccion push %d\n", mem_partition[current_process_index].ptr_stack); 
  mem_partition[current_process_index].ptr_stack -= 1;

  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {

  mem_partition[current_process_index].ptr_stack += 1;
  *out = m_read(mem_partition[current_process_index].ptr_stack);

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  
  int base = mem_partition[current_process_index].base;
  int heap_addr = mem_partition[current_process_index].heap_addr;

  // Verificar que addr sea una dirección que ya haya reservado
  if (belong(mem_partition[current_process_index].heap, addr) || (mem_partition[current_process_index].heap_size < addr && addr <= mem_partition[current_process_index].ptr_stack))
  {
    // printf("hoo");
    return 1;
  }

  *out = m_read(base + heap_addr + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  
  int base = mem_partition[current_process_index].base;
  int heap_addr = mem_partition[current_process_index].heap_addr;

  // Verificar que addr sea una dirección que ya haya reservado
  if (belong(mem_partition[current_process_index].heap, (int)addr) || (mem_partition[current_process_index].heap_size < (int)addr && (int)addr < mem_partition[current_process_index].ptr_stack))
  {
    return 1;
  }
  // printf("holaaa\n");
  m_write(base + heap_addr + addr, val);
  // printf("storeado %i", m_read(base + heap_addr + addr));
  // printf(" de %d\n", val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  
  // Buscar donde se encuentra en la memoria el proceso siguiente
  for (int i = 0; i < total_count_process; i++)
  {
    if(mem_partition[i].pid == process.pid)
    {
      current_process_index = i;
      return;
    }
  }

  for (int i = 0; i < total_count_process; i++)
  {
    if(mem_partition[i].pid == -1)
    {
      // printf("%d to %d\n",1024*i, 1024*i + 1023);
      m_set_owner(1024*i,1024*i + 1023);
      current_process_index = i;
      mem_partition[i].pid = process.pid;
      mem_partition[i].base = 1024*i;
      mem_partition[i].heap = createfree_space(-2,0);
      mem_partition[i].heap_addr = process.program->size;
      mem_partition[i].heap_size = 0;
      mem_partition[i].ptr_stack = 1024*i + 1023;
      return;
    }
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {


  // Buscar el proceso para terminarlo
  for (int i = 0; i < total_count_process; i++)
  {
    if(mem_partition[i].pid == process.pid)
    {
      mem_partition[i].pid = -1;
      // destroid(mem_partition[i].heap);
      // free(mem_partition[i].heap);
      m_unset_owner(1024*i,1024*i + 1023);

      return;
    }
  }
}

void totalfree() {

  // FILE* fichero;
  // fichero = fopen("hola.txt", "a+");
  // fprintf(fichero, "hola %d\n", 0);
  // fclose(fichero);
  free(mem_partition);
}