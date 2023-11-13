#include "pag_manager.h"
#include "Structures/pagStruct.h"

#include "stdio.h"

pagStruct* array_procs;
int* page_frame_mapping; // indica a que page_frame pertenece en la memoria física
int cant_pages = 4; // cantidad de páginas en la memoria virtual
int page_size = 512; // tamaño de las páginas
int curr_proc_index = -1; // Indice del proceso actual en el array
int page_frame_size = -1; // cantidad de páginas en la memora física

// Inicializa el array de page frame
void init_page_frame()
{  
  for (int i = 0; i < page_frame_size; i++) 
  {
    page_frame_mapping[i] = -1;
    array_procs[i].proc_pid = -1;  
    array_procs[i].proc_stack = initialize(10000);
    
    // Inicializar las pages
    for (int j = 0; j < 3; j++) {
        array_procs[i].pages[j] = -1;
    }
  }
}  

// Busca un proceso en el array de procesos
// Devuelve su posición si lo encuentra, else -1
int pag_find_proc(int pid)
{
    for (int i = 0; i < page_frame_size; i++) 
    {
        if (array_procs[i].proc_pid == pid)
            return i; // Devuelve la posición en el array donde está el proceso
    }
    return -1;
}

// Devuelve el primer índice donde no tengo ningún proceso, else -1
int first_empty()
{
    for (int i = 0; i < page_frame_size; i++) 
    {
        if (array_procs[i].proc_pid == -1)
            return i;
    }
    return -1;
}

// Devuelve el primer frame page vacío, else -1
int first_empty_frame() 
{
  for (int i = 0; i < page_frame_size; i++) 
  {
    if (page_frame_mapping[i] == -1)
      return i;
  }
  return -1;
}

// Función para setear el owner según el empty frame encontrado
void set_owner(int emp_fr)
{
  m_set_owner(emp_fr * 512, emp_fr * 512 + 511);
}

// Función para actualizar el mapeo según el empty frame
void set_mapping(int emp_fr)
{
  page_frame_mapping[emp_fr] = array_procs[curr_proc_index].proc_pid;
}

// Función para setear un valor en una página del array de procesos
void set_pages(int ind, int emp_fr)
{
  array_procs[curr_proc_index].pages[ind] = emp_fr;
}

// Función para establecer un valor en una posición de la matriz actual
void set_data(int i, int j, int val)
{
  array_procs[curr_proc_index].data[i][j] = val;
}

// Mapeo a la dirección real
int local_addr_mod(int ind)
{
  return ind % 512;
}

// Mapeo a la dirección real
int page_addr_div(int ind)
{
  return ind / 512;
}

// Función para escribir en el heap
int set_heap_values(int val, int size, int ind)
{
  int page = page_addr_div(ind); 
  int local = local_addr_mod(ind);
  int count = 0;

  for (int i = page; i < 4; i++) 
  {
    int j;

    if (i == page)
      j=local;
    else
      j = 0;

    while (j < 512)
    {
      if (count == size) 
        return 0;
      set_data(i, j, val);
      count++;
      j++;
    }     
  }
  return 0;
}

// 1 => page libre, else 0
int free_page(int page)
{  
  for (int i = 0; i < 512; i++) 
  {
    if (array_procs[curr_proc_index].data[page][i] != 0)
      return 0;
  }
  return 1;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  // Define el tamaño de los page frame
  page_frame_size = m_size() / page_size;
  // Inicializa el array de procesos
  array_procs = init_pagProcs_array(page_frame_size);
  // Almacena memoria para el array de page frame
  // page_frame_mapping = init_page_frame();

  page_frame_mapping = (int*)malloc(page_frame_size * sizeof(int));

  init_page_frame();
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int start = -1;
  size_t count = 0;

  // Recorrer toda la matriz
  for (int i = 0; i < 4; i++) 
  {
    for (int j = 0; j < 512; j++)
    {
      // Caso donde necesito una nueva página
      if (array_procs[curr_proc_index].pages[i] == -1) 
      {
        // Obtengo el primer frame vacío
        int empty_frame = first_empty_frame();
        
        // Si hubo alguno vacío: 
        if (empty_frame != -1) 
        {
          set_mapping(empty_frame);  
          set_pages(i, empty_frame);
          set_owner(empty_frame);
        } 
        else // No hubo ningún frame vacío y no se puede avanzar
          return 1;
      }

      // Caso donde tengo la página vacía
      if (array_procs[curr_proc_index].data[i][j] == 0) 
      {
        count++;
        if (start == -1)
          start = (i + 1) * j;
      } 
      else 
      {
        count = 0;
        start = -1;
      }

      // Si ya tengo el espacio que necesito
      if (count == size) 
      { 
        out->size = size;
        out->addr = start;
        
        set_heap_values(1, size, start);
        return 0;
      }
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  int dir = ptr.addr;
  int size = ptr.size;
  return set_heap_values(0, size, dir);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  // Recorrer la matriz
  for (int i = 0; i < 4; i++) 
  {
    for (int j = 0; j < 512; j++)
    {
      
      // Caso donde se necesita un nuevo page
      if (array_procs[curr_proc_index].pages[i] == -1) 
      {
        int empty_frame = first_empty_frame();
        
        // Si se encontró un frame vacío 
        if (empty_frame != -1) 
        {
          set_mapping(empty_frame);
          set_pages(i, empty_frame);
          set_owner(empty_frame);
        }
        else
          return 1;
      }

      if (array_procs[curr_proc_index].data[i][j] == 0) 
      {
        int index = array_procs[curr_proc_index].pages[i] * 512 + j;
        int value = i * 512 + j;
        
        // Escribe en la memoria
        m_write(index, val);

        // Pusheo el valor a mi stack
        push(array_procs[curr_proc_index].proc_stack, value);
        set_data(i, j, 2);
        
        return 0;
      } 
    }
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  // Obtengo el último elemento del stack sin sacarlo aún
  int dir = get_tail(array_procs[curr_proc_index].proc_stack);
  // Saco el elemento
  pop(array_procs[curr_proc_index].proc_stack);
  
  int local = local_addr_mod(dir);
  int page = page_addr_div(dir);

  set_data(page, local, 0);

  // La página page está libre
  if (free_page(page) == 1) 
  {
    int phs_mem_page = array_procs[curr_proc_index].pages[page];
    page_frame_mapping[phs_mem_page] = -1;
    array_procs[curr_proc_index].pages[page] = -1;
    m_unset_owner((phs_mem_page + 1) * 512, (phs_mem_page + 1) * 512 + 511);
  }
  int a = array_procs[curr_proc_index].pages[page] * 512 + local;
  *out = m_read(a);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) 
{
  int local = local_addr_mod(addr);
  int page = page_addr_div(addr);
  
  if (array_procs[curr_proc_index].pages[page] != -1) 
  {
    if (array_procs[curr_proc_index].data[page][local] == 1) 
    {
      *out = m_read(array_procs[curr_proc_index].pages[page] * 512 + local);
      return 0;
    }
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  int local = local_addr_mod(addr);
  int page = page_addr_div(addr);

  if (array_procs[curr_proc_index].pages[page] != -1) 
  {
    if (array_procs[curr_proc_index].data[page][local] == 1) 
    {
      m_write(array_procs[curr_proc_index].pages[page] * 512 + local, val);
      return 0;
    }
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  
  // Almacena la posición en el array de procesos donde se encuentra el proceso que entró
  int index = pag_find_proc(process.pid);

  if (index == -1)  // Es un proceso nuevo
  {
    int empty_pos = first_empty();
    
    if (empty_pos != -1)  // Hay un espacio del array de procesos libre
    {
      // Establezco como posición del proceso actual esa que encontré vacía
      curr_proc_index = empty_pos;

      // Establezco el pid del proceso nuevo en el array de procesos
      array_procs[curr_proc_index].proc_pid = process.pid;

      // Posición donde se encontró el primer page_frame vacío
      int empty_frame = first_empty_frame();

      // Si fue distinto de -1 es porque se encontró un page_frame vacío 
      if (empty_frame != -1)
      {
        // Establezco el pid del proceso nuevo en el page frame
        set_mapping(empty_frame);
        set_pages(0, empty_frame);
        set_owner(empty_frame);

        // Modificar el frame para escribir el código en él
        int fr_code = process.program->size % 512;
        
        for (int i = 0; i <= fr_code; i++) 
        {
          set_data(0, i, 3);  // lo relleno con un 3 que representa que es código
        }
        return;
      }
    }
    else // No hubo espacios libres y no se puede continuar
    {
      fprintf(stderr, "\nError: no quedan espacios libres.\n");
      exit(1);
    }
  }
  // De lo contrario, no es un proceso nuevo y hay que actualizar el proceso actual
  curr_proc_index = index;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for (int i = 0; i < page_frame_size; i++) 
  {
    if (array_procs[i].proc_pid == process.pid)
      array_procs[i].proc_pid = -1;

    if (page_frame_mapping[i] == process.pid)
      m_unset_owner(i * 512, i * 512 + 511);
  }
  return;
}
