#ifndef BNB_MANAGER_H
#define BNB_MANAGER_H

#include "../memory.h"
#include "../utils.h"

#include "../memory.h"
#include "../utils.h"
#include "stack.h"
#include "process_pag.h"
#include "list.h"

#include "process_list.h"
#define BNB_PAGE_SIZE 512

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv);

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out);

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr);

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out);

// Quita un elemento del stack
int m_bnb_pop(byte *out);

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out);

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val);

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process);

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process);

#endif
