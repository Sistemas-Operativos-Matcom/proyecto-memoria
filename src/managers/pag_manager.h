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

//Agrega un espacio libre al inicio de una free list
void beggining_freelist_pag(node_t** head, addr_t addr, int size);

//Agrega un espacio libre a una free list
void insert_freelist_pag(node_t** head, addr_t addr, int size);

//Elimina un espacio libre de una free list
addr_t less_freelist_pag(node_t** head, int size);

//Inserta un proceso a una lista de procesos
void push_page_item(node_page_item_t** head, page_item_t item);

//Elimina un proceso de una lista de proceso
void delete_page_item(node_page_item_t** head, int pid);

//Agrega una pagina a una lista de paginas
void push_page(page_list_t** head, page_t page);

//Elimina una pagina de una lista de paginas
void delete_page(page_list_t** head, int number);

//Egrega un numero a una lista de enteros
void push_number(number_list_t** head,int number);

//Elimina un entero de una lista de enteros
void delete_number(number_list_t** head, int number);
#endif
