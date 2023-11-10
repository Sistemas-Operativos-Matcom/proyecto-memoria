#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

#define MEM_FAIL 1
#define MEM_SUCCESS 0

#define KB_SIZE(size) size * 1024
#define NO_ONWER -1

typedef unsigned char byte;
typedef byte *memory_t;
typedef size_t addr_t;

// Inicializa la memoria
void mem_init(size_t size, const char *log_file_path);

// Devuelve el tamaño de la memoria
size_t m_size();

// Escribe un valor en una dirección de memoria
void m_write(addr_t addr, byte val);

// Lee el valor que se encuentra en una dirección de memoria
byte m_read(addr_t addr);

// Establece qué parte de la memoria pertenece a un proceso
void m_set_owner(size_t from_addr, size_t to_addr);

// Establece qué parte de la memoria deja pertenece a cualquier proceso
void m_unset_owner(size_t from_addr, size_t to_addr);

void set_curr_owner(int owner);
void mem_end();

#endif // !MEMORY_H
