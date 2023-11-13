#include "stdio.h"
typedef struct bnb_process
{
    int pid;
    int base;
    int bound;
    int stack_pointer;
    int code_size;
    int heap [1024];
    int inst_pointer;
}bnb_process_t;
