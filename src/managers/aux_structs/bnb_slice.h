#ifndef BNB_SLICE_H
#define BNB_SLICE_H

#include "free_list.h"

typedef struct bnb_slice{
    int owner_pid;
    int base, bound;
    int stack_ptr;
    int heap_ptr;
    free_list_t heap;
} bnb_slice_t;

#endif