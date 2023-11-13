#ifndef SHARED_H
#define SHARED_H

#include "../utils.h"

#define INITIAL_FREE_LIST_SIZE 10
#define MAX_SEGMENT_SIZE 64

typedef struct store {
    addr_t address;
    byte value;
} store_t;

// Basic Free List
typedef struct mem_segment {
    addr_t start;
    addr_t end;
} mem_segment_t;

size_t mem_segment_distance(mem_segment_t m1, mem_segment_t m2);

typedef struct free_list {
    int count;
    mem_segment_t *segments;
} free_list_t;

free_list_t *init_free_list();

size_t free_list_end(free_list_t*);
size_t get_free_segment_start(free_list_t*, size_t);
addr_t allocate_segment(free_list_t*, size_t);
void unallocate_segment(free_list_t*, addr_t);

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

// Segmentation free list
typedef struct seg_segment {
    process_t proc;
    addr_t base;
    size_t offset;
    int increase;
    store_t *store;
    int store_count;
    free_list_t *free_list;
} seg_segment_t;

typedef struct seg_free_list {
    int count;
    int max_count;
    seg_segment_t *segments;
} seg_free_list_t;

int seg_get_segment_count(size_t);
seg_segment_t *seg_allocate_segment(seg_free_list_t*, process_t, int);
void seg_unallocate_segment(seg_free_list_t*, int pid);

#endif