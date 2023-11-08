#ifndef SHARED_H
#define SHARED_H

#include "../utils.h"

#define INITIAL_FREE_LIST_SIZE 10

typedef struct store {
    addr_t address;
    byte value;
} store_t;

typedef struct mem_segment {
    addr_t start;
    addr_t end;
} mem_segment_t;

size_t mem_segment_distance(mem_segment_t m1, mem_segment_t m2);

typedef struct free_list {
    int count;
    mem_segment_t *segments;
} free_list_t;

typedef struct bnb_segment {
    process_t proc;
    addr_t base;
    size_t heap_size;
    free_list_t *heap_free_list;
    store_t *heap;
    int heap_count;
    int stack_size;
    store_t *stack;
} bnb_segment_t;


free_list_t *init_free_list();

size_t free_list_end(free_list_t*);
size_t get_free_segment_start(free_list_t*, size_t);
addr_t allocate_segment(free_list_t*, size_t);
void unallocate_segment(free_list_t*, addr_t);

#endif