#include "utils.h"

#include <stdlib.h>

program_t new_program(char *name, size_t size) {
  return (program_t){name, size};
}

process_t new_process(int pid, program_t *program) {

  
  return (process_t){pid, program};
}