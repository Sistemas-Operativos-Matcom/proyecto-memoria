#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include "memory.h"

enum mem {FREE,USED};


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

typedef struct section{

  addr_t real_base;
  size_t size;
  short heap[512];
  int stack_pointer;

}section_t;

program_t new_program(char *name, size_t size);
process_t new_process(int pid, program_t *program);

#endif
