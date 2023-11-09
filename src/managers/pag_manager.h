#ifndef PAG_MANAGER_H
#define PAG_MANAGER_H

#include "../memory.h"
#include "../utils.h"

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv);

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out);

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr);

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out);

// Quita un elemento del stack
int m_pag_pop(byte *out);

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out);

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val);

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process);

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process);

// Pagina y toda su informacion
// pte: direccion para convertir a binario
// valid: si contiene informacion (no se si lo use)
// bits de proteccion: si puede escribir, leer, etc (no se si lo use)
// pfn: numero de pagina fisica
typedef struct page_info {
  int owner;
  int order_owner;
  addr_t data;
  addr_t datainit;
  int *owner_ptr_pg;

} page_info_t;
#endif
