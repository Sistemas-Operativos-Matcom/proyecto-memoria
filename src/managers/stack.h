#ifndef STACK_H
#define STACK_H

#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include <stddef.h>

typedef struct stack
{
    int size_reserve;
    int sp;
} stack_t;

stack_t *init_stack(int size, int program_size);

#endif