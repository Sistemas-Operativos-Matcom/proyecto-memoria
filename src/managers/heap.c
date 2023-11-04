#include "list.h"
#include "../memory.h"
#include "../utils.h"
#include "heap.h"

heap_t *init_heap(process_t process)
{
    heap_t *new_heap = (heap_t *)malloc(sizeof(heap_t));
    sizeList_t *new_list = init();
    new_heap->list = new_list;
    new_heap->start_virtual_pointer = process.program->size;
}