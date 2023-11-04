#ifndef HEAP_H
#define HEAP_H

#include "memory.h"

typedef struct Heap{
    size_t from_addr;
    size_t to_addr;
} Heap_t;

Heap_t Heap_init(size_t from);

#endif