#include "heap.h"

int reserve(size_t size, addr_t stack_limit, Heap_t *heap){
    
    // Searching first free space of size {size}
    for(int i = heap->to_addr; i > (int)stack_limit; i--)
    {
        // Empty slot founded
        if(heap->used_slots[i] == 0)
        {
            for(int j = i; j < (int)i+(int)size; j++)
            {
                // Go back and keep searching
                if(j > (int)heap->from_addr || (int)heap->used_slots[j] != 0) break;

                if(j-i == (int)size-1)
                {
                    // Reserving {size} bytes
                    for(size_t k = 0; k < size; k++)
                    {
                        // 2 in usedSlots array means reserved
                        heap->used_slots[k+i] = 2;
                    }

                    //Updating heap limit if needed
                    if((int)heap->from_addr > i)
                    {
                        heap->from_addr = i;
                    }
                    return i;
                }
            }
        }
    }
    return -1;
}

Heap_t Heap_init(addr_t from, int max_len){
    Heap_t heap;
    int *used_slots = (int*)malloc(max_len*sizeof(int));
    heap.used_slots = used_slots;
    heap.from_addr = from;
    heap.to_addr = from;
    heap.reserve = reserve;

    return heap;
}
   