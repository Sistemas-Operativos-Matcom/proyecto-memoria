#include "bnb_manager.h"
#include "stdio.h"
#include "../memory.h"

static int pid_Actual;              // ID del proceso actual
static addr_t addr_Actual;          // Dirección actual
static addr_t *addr_Procesos;       // direcciones de procesos
static Block_t *memoria_Virtual;   // bloques de memoria

#define tam_Bloque 1024
#define tam_Code 1
#define Kb(size) ((size) / tam_Bloque)

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  
  if (memoria_Virtual != NULL) {
    free(memoria_Virtual);      // Libera la memoria
    memoria_Virtual = NULL;
  }
  
  if (addr_Procesos != NULL) {
    free(addr_Procesos);  // Libera las direcciones de procesos
    addr_Procesos = NULL;
  }

  size_t cant_Bloques = Kb(m_size());
  
  memoria_Virtual = (Block_t *)malloc(sizeof(Block_t) * cant_Bloques);  
  addr_Procesos = (size_t *)malloc(sizeof(size_t) * cant_Bloques); 
  addr_Actual = 0;

  for (size_t i = 0, start = 0; i < cant_Bloques; i++, start += tam_Bloque) {
    Block_t* bloque_Actual = &memoria_Virtual[i];
    bloque_Actual->heap = start + tam_Code;  
    bloque_Actual->stack = start + tam_Bloque - 1;  
    bloque_Actual->start = start + tam_Code;  
    bloque_Actual->end = start + tam_Bloque - 1;  
    bloque_Actual->tam = 0; 
    bloque_Actual->en_uso = 0;
    bloque_Actual->usuario = NO_ONWER;  
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out){
  
   for (size_t i = 0; i < Kb(m_size()); i++) {
    if (!memoria_Virtual[i].en_uso) { 
      size_t desplazado = i * tam_Bloque;
      m_set_owner(desplazado + tam_Code, desplazado + tam_Bloque - 1);      
      //llenar datos de procesos y memoria
      addr_Procesos[pid_Actual] = i; 
      addr_Actual = i;  
      memoria_Virtual[i].en_uso = 1;  
      memoria_Virtual[i].usuario = pid_Actual;  
      memoria_Virtual[i].tam = size;  

      //llenar la salida
      out->addr = desplazado + tam_Code;  
      out->size = 1;
      return 0;  
    }
  }
  return 1; 
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  
  size_t start = memoria_Virtual[addr_Actual].start;
  size_t end = memoria_Virtual[addr_Actual].end;

  if (ptr.addr >=  start && ptr.addr + ptr.size < end ) {  // Comprueba si la dirección pertenece al bloque actual.
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);  
    memoria_Virtual[addr_Actual].tam -= ptr.size;  
    return 0; 
  }

  return 1;  
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  
  if (memoria_Virtual[addr_Actual].stack - 1 <= memoria_Virtual[addr_Actual].heap) {
    return 1;  //pila llena
  }

  //agrega el dato a la pila y cambia la direccion
  m_write(memoria_Virtual[addr_Actual].stack, val);  
  memoria_Virtual[addr_Actual].stack--;  
  out->addr = memoria_Virtual[addr_Actual].stack;  
  return 0;  
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {

  addr_t stack_top = memoria_Virtual[addr_Actual].stack + 1;
  addr_t ini_bloque = memoria_Virtual[addr_Actual].start;
  addr_t fin_bloque = memoria_Virtual[addr_Actual].end;

  if (ini_bloque + fin_bloque <= stack_top) {
    return 1; 
  }

  *out = m_read(stack_top);  // Lee el valor en la cima de la pila
  memoria_Virtual[addr_Actual].stack++;  // Actualiza la posición de la pila
  return 0; 
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  addr_t ini = memoria_Virtual[addr_Actual].start;
  addr_t fin = memoria_Virtual[addr_Actual].end;

  if (addr >= ini && addr < fin) {  // Comprueba si la dirección pertenece al bloque actual.
    *out = m_read(addr);  
    return 0;  
  }

  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  addr_t iniAddress = memoria_Virtual[addr_Actual].start;
  addr_t tamActual = memoria_Virtual[addr_Actual].tam;

  if (addr >= iniAddress && addr < iniAddress + tamActual) {  // Comprueba si la dirección pertenece al bloque actual.
    m_write(addr, val);  
    return 0;  
  }

  return 1;  
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

  pid_Actual = process.pid;  // Actualiza el pid del proceso actual.
  addr_Actual = addr_Procesos[process.pid];  // Actualiza la dirección actual.
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  
  addr_t addr = addr_Procesos[process.pid];  // Obtiene la dirección del proceso.
  m_unset_owner(memoria_Virtual[addr].start, memoria_Virtual[addr].end); 
  memoria_Virtual[addr].en_uso = 0;  
  memoria_Virtual[addr].usuario = NO_ONWER;  
  memoria_Virtual[addr].tam = 0;  
  memoria_Virtual[addr].heap = memoria_Virtual[addr].start; 
  memoria_Virtual[addr].stack = memoria_Virtual[addr].end;  
}
