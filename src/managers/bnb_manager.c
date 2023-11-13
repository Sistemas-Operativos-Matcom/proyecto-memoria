#include "bnb_manager.h"
#include "Structures/bnbStruct.h"

#include "stdio.h"

int process_size = 512; // Tamaño que va a ser asignado a cada proceso
bnbProcess* array_procesos; // Array que contiene los procesos
int cant_procesos;  //Define la cantidad de procesos que se pueden almacenar
int current_process_index = -1; // Pid del proceso actual

// Busca un proceso en el array de procesos por su pid
// Devuelve el indice si encuentra el proceso, else -1
int bnb_find_proc(int pid) 
{
  for (int i = 0; i < cant_procesos; i++)
  {
    if (array_procesos[i].pid_proceso == pid)
      return i;
  }
  return -1;
}

// Devuelve el primer indice del array de procesos donde no hay ningún proceso guardado.
// Si todo está lleno, devuelve -1.
int first_undef_proc()
{
    for (int i = 0; i < cant_procesos; i++)
    {
        if (array_procesos[i].pid_proceso == -1)
            return i;
    }
    return -1;
}

// Reserva un espacio determinado en el heap
void reserve_space(int start, int end)
{
    for (int i = start; i<end; i++)
    {
        array_procesos[current_process_index].heap[i] = 1;
    }
}

// Limpia un espacio determinado en el heap
void free_space(int start, int end)
{
    for (int i = start; i < end; i++)
    {
        array_procesos[current_process_index].heap[i] = 0;
    }
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  // Cantidad de procesos que puedo almacenar según el testcase
  cant_procesos = m_size() / process_size;
  array_procesos = init_bnbProcess_array(cant_procesos);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    // Para facilitar la implementación en este método:
    int m_stack = array_procesos[current_process_index].stack;
    int m_code = array_procesos[current_process_index].code_size;
    int* m_heap = array_procesos[current_process_index].heap;
    int pos = 0;

    //Comienzo al inicio del heap a buscar bloques vacíos. No puedo llegar al stack
    while (m_code + pos < m_stack)
    {
        // Encontré un cero
        if (m_heap[pos] == 0)
        {   
            //Comienzo a contar a partir de esa posición
            int start = pos;
            while (m_heap[pos] == 0)
            {
                pos ++;
                size_t counter = pos-start;
                // Se encontró un bloque de ceros de tamaño suficiente sin llegar al stack
                if (counter == size && pos < m_stack)
                {
                    reserve_space(start, pos); // Reservo el espacio en el heap
                    out->addr = start + m_code;
                    out->size = size;                      
                    return 0;
                }
            }
        }
        // Avanzo la posición porque el bloque encontrado no fue lo suficientemente grande o no se encontró ningún bloque
        pos ++;
    }
    return 1;   // Nunca hubo bloques vacíos del tamaño que se requería
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  free_space(ptr.addr, ptr.addr + ptr.size);
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
    int curr_stack = array_procesos[current_process_index].stack;
    int curr_base = array_procesos[current_process_index].base;
    // Verificar que el stack no esté lleno:
    if (curr_stack >= process_size)
        return 1;

    // Definir la dirección de memoria donde se va a escribir
    addr_t addr = curr_stack + curr_base; 
    // Escribe en esa dirección
    m_write(addr, val);
    // out = m_read(curr_stack + curr_base);
    
    array_procesos[current_process_index].stack--; 
    return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
    int curr_stack = array_procesos[current_process_index].stack;
    // Comprobar si el stack se encuentra vacío
    if (curr_stack >= process_size - 1)
        return 1;
    
    // Leer el valor de la memoria
    *out = m_read(array_procesos[current_process_index].base + curr_stack + 1);
    // Actualizar el puntero del stack del proceso actual
    array_procesos[current_process_index].stack ++;
    return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  // Verificar que el heap tiene algo en esa posición:
  int curr_code_size = array_procesos[current_process_index].code_size;
  if (array_procesos[current_process_index].heap[addr - curr_code_size] == 1)
  {
    *out = m_read(array_procesos[current_process_index].base + addr);
    return 0;
  }
  return 1; // El heap estaba vacío por lo cual no hay nada que cargar
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
    int curr_code_size = array_procesos[current_process_index].code_size;
    int curr_base = array_procesos[current_process_index].base;
    // Verificar que la dirección que se recibe sea válida
    if (array_procesos[current_process_index].heap[addr - curr_code_size] == 1)
    {
        m_write(curr_base + addr, val);
        return 0;
    }
    return 1; // No se encontró nada en esa dirección
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  int new_proc_pid = process.pid;
  int c_size = process.program->size;

  int result = bnb_find_proc(new_proc_pid);
  if (result != -1) // Encontró el proceso en el array
    current_process_index = result;
  else // El proceso no estaba en el array
  {
    int index = first_undef_proc();
    if (index != -1) // Significa que encontró un espacio libre satisfactoriamente
    {
        current_process_index = index;  // Actualizo el índice del proceso actual
        
        // Inicializo valores en el array de procesos
        array_procesos[index].pid_proceso = new_proc_pid;  
        array_procesos[index].code_size = c_size;
        
        bnbProcess current_proc = array_procesos[current_process_index];
        // Le doy a ese proceso lo que le corresponde en la memoria física
        m_set_owner(current_proc.base, current_proc.base + process_size - 1);
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  for (int i = 0; i < cant_procesos; i++) 
  {
    if (array_procesos[i].pid_proceso == process.pid)
    {
      array_procesos[i].pid_proceso = -1;
      m_unset_owner(array_procesos[i].base, array_procesos[i].base + process_size - 1);
    }        
  }  
}
