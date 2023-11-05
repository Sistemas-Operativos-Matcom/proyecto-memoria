#include "heap.h"

int reserve(size_t size, size_t stack_limit, Heap_t *heap){
    
    // Searching first free space of size {size}
    for(int i = heap->to_addr; i > (int)stack_limit; i--)
    {
        // Empty slot founded
        if(heap->used_slots[i] == 0)
        {
            fprintf(stderr, "found in %d, need %ld\n", i, size);
            for(int j = i; j < (int)i+(int)size; j++)
            {
                if(j > (int)heap->from_addr || (int)heap->used_slots[j] != 0) break;


                if(j-i == (int)size-1)
                {
                    // Reserving {size} bytes
                    for(size_t k = 0; k < size; k++)
                    {
                        fprintf(stderr, "reserved %ld\n", k+1);
                        fprintf(stderr, "in %ld\n", i+k);

                        // 2 in usedSlots array means reserved
                        heap->used_slots[k+i] = 2;
                    }

                    //Updating heap limit if needed
                    if((int)heap->from_addr > i)
                    {
                        heap->from_addr = i;
                        fprintf(stderr, "heap->from_addr %ld\n", heap->from_addr);
                    }
                    return 0;
                }
            }
        }
    }
    return 1;
}

Heap_t Heap_init(size_t from, int max_len){
    Heap_t heap;
    int  *used_slots = (int*)malloc(max_len*sizeof(int));
    heap.used_slots = used_slots;
    heap.from_addr = from;
    heap.to_addr = from;
    heap.reserve = reserve;
    return heap;
}
   