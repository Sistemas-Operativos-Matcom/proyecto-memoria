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

//inserta en la free list
void pag_free_insert(free_list_t** head, addr_t addr, int size);
//elimina en la free list y devuelve el addr de donde ocupo espacio
addr_t pag_free_delete(free_list_t** head, int size);

//inserta un proceso en la linked list
void pag_ins_procs (pag_procs_list_t ** head, pag_procs_t procs);
//elimina un proceso en la linked list
void pag_del_procs(pag_procs_list_t ** head, int pid);

//insertar pagina
void pag_ins_pag(page_list_t** head, page_t pag);
//elimina pagina
void pag_del_pag(page_list_t** head, int num);

//inserta un entero
void int_ins(int_l_t** head, int number);
//elimina un entero
void int_del(int_l_t** head, int number);

#endif
