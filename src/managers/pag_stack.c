#include "pag_stack.h"
#include "../memory.h"
#include "../utils.h"
#include "pag_manager.h"

int pag_reserve(size_t size, Pag_Heap_t *heap)
{
    for(int p = 0; p < 5; p++)
    {
        // There is space to save a variable of size {size}
        if(heap->pages_bound[p]-heap->len[p] >= (int)size)
        {
            for(int i = 0; i < (int)size; i++)
            {
                // 2 in used_slots array means reserved
                heap->used_slots[p][heap->len[p]+i] = 2;
            }
            heap->len[p] += size;
            return heap->len[p] - size;
        }
        if(heap->len[p] == -1)
        {
            fprintf(stderr, "entre\n");
            // Allocating new page of heap
            int heap_size = 512;
            int heap_start = find_free_space(heap_size);
            if(heap_start == -1)
            {
                for(int i = heap_size-1; i > 0; i--)
                {
                    heap_start = find_free_space(i);
                    if(heap_start != -1)
                    {
                        heap_size = i;
                        break;
                    } 
                }
            }
            m_set_owner(heap_start, heap_start+heap_size);
            
            heap->used_slots[p] = (int*)malloc(heap_size*sizeof(int*));
            heap->pages_base[p] = heap_start;
            heap->pages_bound[p] = heap_start+heap_size;
            for(int i = 0; i < (int)size; i++)
            {
                // reserving
                heap->used_slots[p][heap_start+i] = 2;
            }
            heap->len[p] = heap_start+size;
            return heap_start;
        }
    }
    return -1;
}

int define_page(addr_t addr, Pag_Heap_t *heap)
{
    for(int p = 0; p < 5; p++)
    {
        if(heap->pages_base[p] <= (int)addr 
        && heap->pages_bound[p]>= (int)addr) return p;
    }
    return -1;
}
int pag_store(addr_t addr, byte val, Pag_Heap_t *heap)
{
    int page = define_page(addr, heap);
    if(page == -1) return 1;

    // Busy or not allocated address
    if(heap->used_slots[page][addr] == 1 || heap->used_slots[page][addr] == 0) return 1;

    // Write val in memory
    m_write(addr, val);
    heap->used_slots[page][addr] = 1;
    return 0;
}

int pag_load(addr_t addr, byte *out, Pag_Heap_t *heap)
{
    int page = define_page(addr, heap);
    if(page == -1) return 1;

    // Empty address, can't be loaded
    if(heap->used_slots[page][addr] == 0 || heap->used_slots[page][addr] == 2) return 1;

    // Copying value in addr to out
    *out = m_read(addr);
    heap->used_slots[page][addr] = 0;
    return 0;
}

Pag_Heap_t Pag_Heap_init(int base, int size)
{
    Pag_Heap_t heap;
    heap.used_slots = (int**)malloc(sizeof(int**));
    heap.used_slots[0] = (int*)malloc(size*sizeof(int*));
    heap.pages_base = (int*)malloc(5* sizeof(int*));
    heap.pages_bound = (int*)malloc(5* sizeof(int*));
    heap.len = (int*)malloc(5* sizeof(int*));
    for(int i = 0; i < 5; i++)
    {
        heap.len[i] = -1;
    }
    heap.pages_base[0] = base;
    heap.pages_bound[0] = base+size;
    heap.len[0] = base;
    heap.pag_reserve = pag_reserve;
    heap.pag_store = pag_store;
    heap.pag_load = pag_load;
    return heap;
}


int pag_push(byte *val, ptr_t *out, Pag_Stack_t *stack)
{
    for(int p = 0; p < 5; p++)
    {
        // Writing in created page
        if(stack->SP[p]+1 != stack->pages_bound[p])
        {
            // Writing val in stack
            m_write(stack->SP[p]+1, *val);
            stack->SP[p] += 1;
            out->addr = stack->SP[p];
            out->size = sizeof(byte);
            return 0;
        }
        // Creating new page to stack
        if(stack->SP[p] == -1)
        {
            int size = 512;
            int base;
            for(int i = size; i > 0; i--)
            {
                base = find_free_space(i);
                if(base != -1)
                {
                    size = i;
                    break;
                } 
            }
            stack->pages_base[p] = base;
            stack->SP[p] = base;
            stack->pages_base[p] = base+size;
            m_set_owner(base, base+size);

            // Writing in page recently created
            m_write(stack->SP[p]+1, *val);
            stack->SP[p] += 1;
            out->addr = stack->SP[p];
            out->size = sizeof(byte);
            return 0;
        }
    }
    return 1;
}

int pag_pop(byte *out, Pag_Stack_t *stack)
{   
    int page = -1;
    for(int p = 0; p < 5; p++)
    {
        if(stack->SP[p] == -1)
        {
            page = p-1;
            break;
        } 
    }
    if(page == -1) return 1;
    
    // Checking pop can be made
    if(stack->pages_bound[page] == stack->pages_base[page]-1) return 1;
    
    // Reading val from stack
    *out = m_read(stack->SP[page]);
    stack->SP[page] -= 1;
    return 0;
}

Pag_Stack_t Pag_Stack_init(int from, int size){

    Pag_Stack_t stack;
    stack.pages_base = (int*)malloc(5*sizeof(int*));
    stack.pages_bound = (int*)malloc(5*sizeof(int*));
    stack.SP = (int*)malloc(5*sizeof(int*));
    for(int i = 0; i < 5; i++)
    {
        stack.SP[i] = -1;
    }
    stack.pages_base[0] = from;
    stack.pages_bound[0] = from+size;
    stack.SP[0] = from;
    stack.pag_push = pag_push;
    stack.pag_pop = pag_pop;
    
    return stack;
}