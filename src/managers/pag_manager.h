#ifndef PAG_MANAGER_H
#define PAG_MANAGER_H

#include "../memory.h"
#include "../utils.h"

typedef struct casilla {
  addr_t addr;
  size_t tam;
}casilla_t;

typedef struct list {
  casilla_t *celda;
  int actual;
  int size;
} list_t;

typedef struct proceso{
  size_t codigo[7];//guarda la direccion donde inician todas las paginas donde se encuentra el codigo
  size_t stack[3];//lo mismo pero para el stack
  int act_stack;//cantidad de paginas que ha usado el stack;
  size_t dir_stack;// direccion actual del stack
  size_t heap[3];//guarda la direccion donde inician todas las paginas donde se encuentra el heap
  list_t heapes[3];// por cada pagina se guarda una free list para saber que espacio esta libre en cada una
  int pid;//pid del proceso
}proceso_t;

void init();

addr_t reservar_memory(size_t tam);

void liberar_memory(addr_t direc, size_t tam);

addr_t res_memory(size_t tam, list_t l);

int lib_memory(list_t l, ptr_t ptr);

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

#endif
