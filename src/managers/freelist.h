#ifndef __FREELIST
#define __FREELIST

#include "stddef.h"

struct FreelistNode{
    size_t begining;
    size_t size;
    struct FreelistNode* next;
};

struct Freelist{
    size_t heap_begining;
    size_t heap_size;
    struct FreelistNode* first;
};

struct Freelist* freelist_init(size_t heap_begining, size_t heap_initial_size);
int freelist_alloc_space(struct Freelist* freelist, size_t size, size_t* out_addr);
void freelist_try_merge_nodes(struct FreelistNode* from);
void freelist_free_space(struct Freelist* freelist, size_t begining, size_t size);
void freelist_grow(struct Freelist* freelist, int d_size);
void freelist_deinit(struct Freelist* freelist);
int debug_freelist_node_count(struct Freelist* freelist);

#endif


