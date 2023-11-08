#include "bnb_manager.h"
#include "shared.h"

#include "stdio.h"

#define BOUNDS_SIZE KB_SIZE(1)
#define MAX_PROCS_STORED m_size() / BOUNDS_SIZE

// Storing the bnb segments
static bnb_segment_t *segments;
static int segment_count;

// Current pid and current segment caching
static int curr_pid;
static int curr_segment_index;

// Checks if a process is allocated on a segment
static int is_process_allocated(int pid) {
    for (int i = 0; i < segment_count; i++) {
        if (segments[i].proc.pid == pid) return 1;
    }

    return 0;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
    segments = malloc(MAX_PROCS_STORED * sizeof(bnb_segment_t));
    segment_count = 0;
    curr_pid = -1;
    curr_segment_index = -1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    bnb_segment_t *segment = &segments[curr_segment_index];
    // Upper bound for stack
    addr_t stack_end = segment->base + BOUNDS_SIZE - segment->stack_size + sizeof(byte);

    // Heap-based virtual address
    addr_t h_addr = allocate_segment(segment->heap_free_list, size);
    // Heap start virtual address
    addr_t v_addr = segment->base + segment->proc.program->size;

    // Checks the allocated memory does not overflows the base + bounds
    if (v_addr + h_addr + size > segment->base + BOUNDS_SIZE) {
        unallocate_segment(segment->heap_free_list, h_addr);
        return MEM_FAIL;
    }

    // Checks the allocated memory does not collide with allocated stack
    if (h_addr + size >= stack_end) {
        unallocate_segment(segment->heap_free_list, h_addr);
        return MEM_FAIL;
    }

    segment->heap_size += size;

    out->addr = v_addr + h_addr;
    out->size = size;
    
    return MEM_SUCCESS;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
    bnb_segment_t *segment = &segments[curr_segment_index];

    // Unallocate the heap segment starting from the given address 
    unallocate_segment(segment->heap_free_list, ptr.addr);

    // Clear the heap store for any value stored inside the segment bounds
    for (int i = 0; i < segment->heap_count; i++) {
        if (segment->heap[i].address >= ptr.addr && segment->heap[i].address < ptr.addr + ptr.size) {
        segment->heap_count--;

        for (int j = i; j < segment->heap_count; j++) {
            segment->heap[j] = segment->heap[j + 1];
        }

        void *status = realloc(segment->heap, segment->heap_count);

        if (status == NULL) return MEM_FAIL;
        }
    }

    // Sets the heap size to the maximum upper bound from the free list
    segment->heap_size = free_list_end(segment->heap_free_list);

    return MEM_SUCCESS;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
    bnb_segment_t *segment = &segments[curr_segment_index];

    // End of the stack address
    addr_t stack_end = segment->base + BOUNDS_SIZE - segment->stack_size + sizeof(byte);
    // End of the heap address
    addr_t heap_end = segment->base + segment->proc.program->size + segment->heap_size;

    // Check stack and heap do not collide
    if (heap_end > stack_end) return MEM_FAIL;

    store_t store = {.address = stack_end, .value = val};

    // Initialize stack if null or reallocate space for new values
    if (segment->stack == NULL) {
        segment->stack = malloc(sizeof(store_t));
    } else {
        void *status = realloc(segment->stack, (segment->stack_size + 1) * sizeof(store_t));

        if (status == NULL) return MEM_FAIL;
    }

    segment->stack[segment->stack_size] = store;
    segment->stack_size++;

    out->addr = store.address;
    out->size = sizeof(byte);

    return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
    bnb_segment_t *segment = &segments[curr_segment_index];
    store_t *store = &segment->stack[segment->stack_size - 1];

    *out = store->value;

    // Check for avoiding false positive 0 length NULL pointer on realloc
    if (segment->stack_size - 1 > 0) {
        void *status = realloc(segment->stack, (segment->stack_size - 1) * sizeof(store_t));

        if (status == NULL) return MEM_FAIL;
    }

    segment->stack_size--;

    return MEM_SUCCESS;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
    bnb_segment_t *segment = &segments[curr_segment_index];

    // Get the stored value at the given address
    for (int i = 0; i < segment->heap_count; i++) {
        store_t *store = &segment->heap[i];

        if (store->address == addr) {
            *out = store->value;

            return MEM_SUCCESS;
        }
    }

    return MEM_FAIL;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
    bnb_segment_t *segment = &segments[curr_segment_index];

    // Check address inside segment bounds
    if (addr > segment->base + BOUNDS_SIZE) return MEM_FAIL;
    // Check address inside heap bounds
    if (addr > segment->base + segment->proc.program->size + segment->heap_size) return MEM_FAIL;

    store_t store = {.address = addr, .value = val};

    // Initialize heap if NULL or reallocate space otherwise
    if (segment->heap == NULL) {
        segment->heap = malloc(sizeof(store_t));
    } else {
        void *status = realloc(segment->heap, (segment->heap_count + 1) * sizeof(store_t));
        
        if (status == NULL) return MEM_FAIL;
    }

    segment->heap[segment->heap_count] = store;
    segment->heap_count++;

    return MEM_SUCCESS;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
    // If the process is not running, allocate a bnb segment for it
    if (!is_process_allocated(process.pid)) {
        bnb_segment_t new_segment = {
            .proc = process,
            .base = segment_count * BOUNDS_SIZE + 1,
            .heap_free_list = init_free_list(),
            .heap_size = 0,
            .heap_count = 0,
            .stack_size = 0
        };

        segments[segment_count] = new_segment;
        segment_count++;
    }

    // Update global variables of cached index and current pid
    int index = -1;

    for (int i = 0; i < segment_count; i++) {
        if (segments[i].proc.pid == process.pid) {
        index = i;
        break;
        }
    }
    
    curr_pid = process.pid;
    curr_segment_index = index;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
    // Remove the segment containing the process and remove any white space between 2 segments after that
    for (int i = 0; i < segment_count; i++) {
        if (segments[i].proc.pid == process.pid) {
            bnb_segment_t empty_segment = {};
            segments[i] = empty_segment;

            for (int j = i; j < segment_count - 1; j++) {
                segments[j] = segments[j + 1];
            }

            if (curr_pid == process.pid) {
                curr_pid = -1;
                curr_segment_index = -1;
            }

            segment_count--;
            return;
        }
    }
}
