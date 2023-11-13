#ifndef BNB_MANAGER_H
#define BNB_MANAGER_H

#include "../memory.h"
#include "../utils.h"

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


typedef struct Block
{
    size_t heap;
    size_t stack;
    size_t size;
    size_t s_addr;
    size_t e_addr;

    int owner;
    int in_use;
} Block;

typedef struct FreeBlock {
  size_t start_addr;
  size_t end_addr;
  struct FreeBlock* next;
} FreeBlock;

#endif
