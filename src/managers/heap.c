#include "heap.h"

Heap_t Heap_init(size_t from){
    Heap_t heap;
    heap.from_addr = from;
    heap.to_addr = from;
    return heap;
}
