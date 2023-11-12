#ifndef MANAGER_H
#define MANAGER_H

#include "memory.h"
#include "utils.h"

// Esta función se llama cuando se inicializa un caso de prueba
void m_init(int argc, char **argv);

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_malloc(size_t size, ptr_t *out);

// Libera un espacio de memoria dado un puntero.
int m_free(ptr_t ptr);

// Agrega un elemento al stack
int m_push(byte val, ptr_t *out);

// Quita un elemento del stack
int m_pop(byte *out);

// Carga el valor en una dirección determinada
int m_load(addr_t addr, byte *out);

// Almacena un valor en una dirección determinada
int m_store(addr_t addr, byte val);

// Notifica un cambio de contexto al proceso 'next_pid'
void m_on_ctx_switch(process_t process);

// Notifica que un proceso ya terminó su ejecución
void m_on_end_process(process_t process);

void reset_values();

#endif
