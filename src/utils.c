#include "utils.h"

#include <stdlib.h>

program_t new_program(char *name, size_t size) {
  return (program_t){name, size};
}

process_t new_process(int pid, program_t *program) {
  return (process_t){pid, program};
}

//Convierte el tamaño del proceso a multiplo de 8
size_t round_proccess_size(size_t size)
{ size_t tempsize = 8;
  while(size/tempsize != 0)
    tempsize *= 2;
  return tempsize;
}