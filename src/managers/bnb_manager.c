#include "bnb_manager.h"
#include "stdio.h"
#include "aux_structs/bnb_slice.h"

#define MAX_PROC_COUNT 0xa

static bnb_slice_t* slices;
static unsigned char is_init = 0;
static int last_assigned = -1;
static size_t slice_size = 0;
static int curr_slice = -1;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  // Limpiar los slices si ya han sido asignados
  if (is_init) free(slices);
  
  // La memoria se divide en segmentos iguales
  slice_size = m_size() / MAX_PROC_COUNT;
  slices = (bnb_slice_t*) malloc(MAX_PROC_COUNT * sizeof(bnb_slice_t)); 
  
  // El ultimo slice asignado no existe, por tanto -1
  last_assigned = -1;
  curr_slice = -1;
  is_init = 1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  free_list_t* list = &slices[curr_slice].heap;
  size_t _out = -1;

  // Intentar obtener la memoria
  int status = fl_get_memory(list, size, &_out);

  // Guardar la direccion obtenida en out
  out->addr = (size_t) _out;

  // status representa si hubo exito o fracaso asi que se retorna
  return status;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  free_list_t* list = &slices[curr_slice].heap;
  
  // Intentar liberar la memoria
  int status = fl_free_memory(list, ptr.size, (int)ptr.addr);

  // Retornar el resultado de la operacion
  return status; 
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  size_t v_sp = slices[curr_slice].stack_ptr;
  size_t v_hp = slices[curr_slice].heap_ptr;

  // Si el stack point va mas alla del tamaño del heap, error
  if (v_hp >= v_sp - 1) {
    return MEM_FAIL;
  }

  // Obtener la direccion fisica del byte a guardar en la pila
  size_t p_sp = slices[curr_slice].base + (-- v_sp);
  slices[curr_slice].stack_ptr --;

  // Guardar el valor en la memoria
  m_write(p_sp, val);

  // Actualizar el puntero de salida
  out->addr = v_sp;
  out->size = 1;

  // Retornar con exito
  return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  int v_sp = slices[curr_slice].stack_ptr;
  
  // Si el sp se pasa del tamaño maximo, error
  if (v_sp + 1 > (int) slice_size) {
    return MEM_FAIL;
  }

  int _addr = v_sp + slices[curr_slice].base;
  
  *out = m_read(_addr);
  slices[curr_slice].stack_ptr ++;

  return MEM_SUCCESS;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  // Si la direccion no pertenece al slice, error
  if (addr >= (addr_t) slices[curr_slice].bound) return MEM_FAIL;

  // Obtener la direccion fisica que se desea obtener
  int _addr = (int) addr + slices[curr_slice].base;
  
  // Asignar el valor buscado en out
  *out = m_read(_addr);

  // Retornar con exito
  return MEM_SUCCESS;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  // Si la direccion no pertenece al slice, error
  if (addr >= (addr_t) slices[curr_slice].bound) return MEM_FAIL;

  // Obtener la direccion fisica que se desea obtener
  int _addr = (int) addr + slices[curr_slice].base;

  // Asignar el valor en la direccion correcta
  m_write(_addr, val);

  // Retornar con exito
  return MEM_SUCCESS;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  int last_empty = -1;
  // Buscar el proceso en la lista de slices
  for (int i = 0; i <= last_assigned; i++) {
    // Si se encuentra se retorna
    if (slices[i].owner_pid == process.pid) {
      curr_slice = i;
      return;
    }
    //Si hay un espacio vacio se guarda
    else if (slices[i].owner_pid == -1) {
      last_empty = i;
    }
  }

  int p = last_empty;

  // Si hay un espacio vacio, se coloca ahi
  // Si no existe, se crea uno nuevo en last_assigned
  if (last_empty == -1) {
    p = ++ last_assigned;
    
    // Resetear el heap
    fl_reset_list(&slices[p].heap, slice_size / 2);
  }
  else {
    // Inicializar el heap como un nuevo free list
    fl_init_list(&slices[p].heap, slice_size / 2);
  }

  slices[p].owner_pid = process.pid;
  slices[p].base = last_assigned * slice_size;
  slices[p].bound = slice_size;
  slices[p].stack_ptr = slice_size;
  // La mitad del espacio es para el heap(decision del arbitro)
  slices[p].heap_ptr = slice_size / 2; 

  // Guardar el proceso como proceso actual
  curr_slice = p;

  m_set_owner(slices[p].base, slices[p].base + slices[p].bound);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  // Buscamos el pid en los slices y se asigna a -1
  for (int i = 0; i <= last_assigned; i++) {
    if (slices[i].owner_pid == process.pid) {
      m_unset_owner(slices[i].base, slices[i].base + slices[i].bound);
      slices[i].owner_pid = -1;
      return;
    }
  }

  fprintf(stderr, "Error while deleting process in BnB.\n");
  exit(1);
}
