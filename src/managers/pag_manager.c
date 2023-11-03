#include "pag_manager.h"
#include "./stdio.h"
#include "stdlib.h"


// Estructura para representar un espacio del heap
typedef struct free_space {
    int base;
    int bound;
    struct free_space* next;
} free_space;

// Función para crear un nuevo espacio
free_space* createfree_space_pag(int base, int bound) {
    free_space* newfree_space = (free_space*)malloc(sizeof(free_space));
    newfree_space->base = base;
    newfree_space->bound = bound;
    newfree_space->next = NULL;
    return newfree_space;
}

// Función para agrupar los espacios vacios adyacentes
void zip_pag(free_space* temp) {
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
int reserve_pag(free_space* heap, int bound) {
  

  // No hay espacios libres en el heap
  if (heap->base == -2) {
      return -1;
  }

  // Buscar el lugar para ocupar
  while (heap != NULL)
  {

    if(heap->bound > bound)
    {
      heap->bound -= bound;
      return heap->bound;
    }

    heap = heap->next;
  }

  return -1;
}

// Pregunta si esa direccion pertenece a algun espacio libre
int belong_pag(free_space* heap, int addr) {

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

// Pregunta en la free list si hay un espacio vacio de un size especifico
int isFree_pag(free_space* heap, int base, int bound) {

  if (heap == NULL) {
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
int insert_pag(free_space* heap, int base, int bound) {
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

// Funcion para eliminar la free list
void destroid_pag(free_space* heap) {
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
  free_space* heap_freeLists[32];
  int heap_pages[32];
  int stack_pages[32];
  int stack_pointer;
} t_context;


// Variables Globales
int* phisic_memory;  // direcciones de la memoria fisica
t_context* processes; // array de contextos de los procesos en curso
int phisic_memory_total; // cantidad de pages frames de la memoria fisica
int current_process_index_pag;

// Funcion para pedir una nueva pageframe
int reserve_pageframe() {

  for (int i = 0; i < phisic_memory_total; i++)
  {
    if (phisic_memory[i] == -1)
    {
      phisic_memory[i] = processes[current_process_index_pag].pid;
      m_set_owner(i*32, i*32 + 31);
      return i;
    }
  }

  // VALIDAR QUE HACER CON ESTE RETORNO 
  return -1;
  
}

// Liberar una pageframe
void free_pageframe(int page_index) {
  
  phisic_memory[page_index] = -1;
  m_unset_owner(page_index*32, page_index*32 + 31);

}






// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  
  current_process_index_pag = -1;
  int memory = m_size(); // memoria total
  phisic_memory_total = (int)(memory/32);
  phisic_memory = (int*)malloc(sizeof(int)*phisic_memory_total);

  for (int i = 0; i < phisic_memory_total; i++)
  {
    phisic_memory[i] = -1;
  }
  

  // Inicializando la lista de contextos
  processes = (t_context*)malloc(sizeof(t_context)*10);
  
  for (int i = 0; i < 10; i++)
  {
    processes[i].pid = -1;
  }

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {

  int heap_page_index = 0;
  while (heap_page_index < 32 && processes[current_process_index_pag].heap_freeLists[heap_page_index]->base != -2)
  {

    int addr = reserve_pag(processes[current_process_index_pag].heap_freeLists[heap_page_index], size);

    if (addr != -1)
    {
      out->addr = 32*heap_page_index + addr;
      return 0;
    }

    heap_page_index++;
  }

  if (heap_page_index == 32)
  {
    return 1;
  }

  processes[current_process_index_pag].heap_pages[heap_page_index] = reserve_pageframe();
  processes[current_process_index_pag].heap_freeLists[heap_page_index] = createfree_space_pag(0, 32-size);

  out->addr = 32*heap_page_index + 32-size;

  return 0;
  
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  
  int page_index = (int)(processes[current_process_index_pag].stack_pointer/32);

  if (processes[current_process_index_pag].stack_pages[page_index] == -1)
  {
    processes[current_process_index_pag].stack_pages[page_index] = reserve_pageframe();
  }
  
  int page = processes[current_process_index_pag].stack_pages[page_index];
  int offset = (int)(processes[current_process_index_pag].stack_pointer % 32);

  m_write(32*page + offset, val);

  processes[current_process_index_pag].stack_pointer++;
    
  // FILE* fichero;
  // fichero = fopen("hola.txt", "a");
  // fprintf(fichero, "direccion %d\n", 32*page + offset);
  // fclose(fichero);
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  
  processes[current_process_index_pag].stack_pointer--;

  int stack = processes[current_process_index_pag].stack_pointer;
  if (stack % 32 == 31)
  {
    free_pageframe((stack + 1)/32);
  }

  int page = processes[current_process_index_pag].stack_pages[stack/32];

  *out = m_read(32*page + stack%32);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  
  int page = processes[current_process_index_pag].heap_pages[(int)(addr/32)];

  *out = m_read(32*page + (addr % 32));

  return 0;
  
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  
  // Verificar que la pagina a la que pertenece la direccion
  // haya sido reservada por el proceso
  int page = processes[current_process_index_pag].heap_pages[(int)(addr/32)];
  if (phisic_memory[page] != processes[current_process_index_pag].pid)
  {
    return 1;
  }

  // Verificar que la direccion no es un espacio libre en el heap
  if (belong_pag(processes[current_process_index_pag].heap_freeLists[(int)(addr/32)],addr))
  {
    // FILE* fichero;
    // fichero = fopen("hola.txt", "a");
    // fprintf(fichero, "hola %d\n", (int)(addr/32));
    // fclose(fichero);
    return 1;
  }
    // FILE* fichero;
    // fichero = fopen("hola.txt", "a");
    // fprintf(fichero, "hola %d\n", page);
    // fclose(fichero);

  m_write(32*page + (addr % 32), val);
  return 0;
  
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  
  // Si el proceso ya fue inicializado cambiar de contexto
  for (size_t i = 0; i < 10; i++)
  {
    if (processes[i].pid == process.pid)
    {
      current_process_index_pag = i;
      return;
    }
  }

  // En caso que el proceso sea nuevo
  for (int i = 0; i < 10; i++)
  {
    if (processes[i].pid == -1)
    {
      current_process_index_pag = i;
      // Inicializar el contexto
      processes[i].pid = process.pid;
      processes[i].stack_pointer = 0;

      for (int j = 0; j < 32; j++)
      {
        processes[i].heap_freeLists[j] = createfree_space_pag(-2,0);
      }
      processes[i].heap_freeLists[0] = createfree_space_pag(0,31);

      for (int j = 0; j < 32; j++)
      {
        processes[i].heap_pages[j] = -1;
      }
      processes[i].heap_pages[0] = reserve_pageframe();
      
      for (int j = 0; j < 32; j++)
      {
        processes[i].stack_pages[j] = -1;
      }
      processes[i].stack_pages[0] = reserve_pageframe();
      
      return;
    }
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  
  for (int i = 0; i < 10; i++)
  {
    if (processes[i].pid == process.pid)
    {
      int count = 0;
      // while (processes[i].heap_freeLists[count]->base != 2)
      // {
      //   destroid_pag(processes[i].heap_freeLists[count]);
      //   count++;
      // }

      count = 0;
      while (processes[i].heap_pages[count] != -1)
      {
        free_pageframe(processes[i].heap_pages[count]);
        count++;
      }

      count = 0;
      while (processes[i].stack_pages[count] != -1)
      {
        free_pageframe(processes[i].stack_pages[count]);
        count++;
      }
      
      processes[i].pid = -1;
      return;
    }
    
  }
  
}

// Liberar estructuras reservadas en pag
void m_pag_totalfree() {

  free(processes);
  free(phisic_memory);
}

