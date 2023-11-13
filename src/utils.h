#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include "memory.h"

// Esta estructura representa un puntero. No puedes cambiar el nombre ni
// eliminar la estructura. Puedes agregar campos nuevos si es necesario.
typedef struct ptr {
  addr_t addr;  // No eliminar este campo
  size_t size;
} ptr_t;

typedef struct program {
  char *name;
  size_t size;
} program_t;

typedef struct process {
  int pid;
  program_t *program;
} process_t;

typedef struct space{
  addr_t heap;
  addr_t stack;
  programSpaceData_t program;
  int is_used;
} space_t;

typedef struct programSpaceData{
  int pid;
  size_t size;
  addr_t base;
  addr_t bound;
} programSpaceData_t;

typedef struct process_info{
  int pid;
  addr_t *page_table;
  addr_t heap;
  addr_t stack;
  int is_used;
} process_info_t;


program_t new_program(char *name, size_t size);
process_t new_process(int pid, program_t *program);

#endif
