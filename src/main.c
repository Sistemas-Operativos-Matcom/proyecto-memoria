#include <stdio.h>
#include <stdlib.h>

#include "tests.h"


int main(int argc, char **argv) {

  
  if (argc < 2) {
    fprintf(stderr, "You must give a memory manager: bnb, seg or pag\n");
    exit(1);
  }
  run_tests(argc, argv);
  return 0;
}