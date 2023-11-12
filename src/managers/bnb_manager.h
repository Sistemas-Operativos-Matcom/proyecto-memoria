#ifndef BNB_MANAGER_H
#define BNB_MANAGER_H

#include "../memory.h"
#include "../utils.h"
#include <stdio.h>
#include <stdlib.h>





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

//Agrega un proceso a la lista de procesos
void push_item(node_item_t** head, item_t item);

//Elimina un proceso a la lista de procesos
void delete_item(node_item_t** head, int pid);

//Agrega un espacio libre al inicio de una free list
void beggining_freelist(node_t** head, addr_t addr, int size);

//Agrega un espacio libre a una free list
void insert_freelist(node_t** head, addr_t addr, int size);

//Elimina un espacio libre de una free list
addr_t less_freelist(node_t** head, int size);

//Maneja la free list luego de hacer un push
int push_freelist(node_t** head,addr_t* stack_pointer);

//Maneja la free list luego de hacer un pop
int pop_freelist(node_t** head,addr_t* stack_pointer, byte* out, int process_end,addr_t base);


#endif
