#include "bnb_manager.h"
#include "stdio.h"

// Estructura que representa el estado actual de un proceso
typedef struct curr_process {
    addr_t heap;             // Puntero al inicio del heap
    addr_t stack;            // Puntero al inicio del stack
    size_t size;             // Tamaño actual del bloque de memoria
    addr_t base;             // Dirección base del bloque de memoria
    addr_t bound;            // Dirección tope del bloque de memoria
    int pid;                 // ID del proceso
    int in_execution;        // Indica si el proceso está en ejecución
} curr_process_t;

// Variables globales
static int curr_pid;              // ID del proceso actual
static addr_t curr_addr;          // Dirección actual
static addr_t *procs_addr;        // Direcciones de procesos
static curr_process_t *blocks; // Bloques de memoria

#define proc_size 1028             // Tamaño de bloque en bytes

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
    free(blocks);      // Libera la memoria
    free(procs_addr);     // Libera las direcciones de procesos

    size_t total_blocks = m_size() / proc_size;

    blocks = (curr_process_t *)malloc(sizeof(curr_process_t) * total_blocks);
    procs_addr = (size_t *)malloc(sizeof(size_t) * total_blocks);
    curr_addr = 0;

    for (size_t i = 0, start = 0; i < total_blocks; i++, start += proc_size) {
        curr_process_t *curr_block = &blocks[i];
        curr_block->heap = start + 1;
        curr_block->stack = start + proc_size - 1;
        curr_block->base = start + 1;
        curr_block->bound = start + proc_size - 1;
        curr_block->size = 0;
        curr_block->in_execution = 0;
        curr_block->pid = NO_ONWER;
    }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    for (size_t i = 0; i < m_size() / proc_size; i++) {
        if (!blocks[i].in_execution) {
            size_t tmp = i * proc_size;
            m_set_owner(tmp + 1, tmp + proc_size - 1);
            // Llenar datos de procesos y memoria
            procs_addr[curr_pid] = i;
            curr_addr = i;
            blocks[i].in_execution = 1;
            blocks[i].pid = curr_pid;
            blocks[i].size = size;

            // Llenar la salida
            out->addr = tmp + 1;
            out->size = 1;
            return 0;
        }
    }
    return 1; 
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
    size_t base = blocks[curr_addr].base;
    size_t bound = blocks[curr_addr].bound;

    if (ptr.addr >= base && ptr.addr + ptr.size < bound) {
        m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
        blocks[curr_addr].size -= ptr.size;
        return 0;
    }

    return 1; 
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
    if (blocks[curr_addr].stack - 1 <= blocks[curr_addr].heap) {
        return 1; // Error: pila llena, no se puede agregar más elementos.
    }

    // Agrega el dato a la pila y cambia la dirección
    m_write(blocks[curr_addr].stack, val);
    blocks[curr_addr].stack--;
    out->addr = blocks[curr_addr].stack;
    return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
    addr_t stack_top = blocks[curr_addr].stack + 1;
    addr_t curr_base = blocks[curr_addr].base;
    addr_t curr_bound = blocks[curr_addr].bound;

    if (curr_base + curr_bound <= stack_top) {
        return 1; // Error: pila vacía, no se puede quitar más elementos.
    }

    *out = m_read(stack_top); // Lee el valor en la cima de la pila
    blocks[curr_addr].stack++; // Actualiza la posición de la pila
    return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
    addr_t curr_base = blocks[curr_addr].base;
    addr_t curr_bound = blocks[curr_addr].bound;

    if (addr >= curr_base && addr < curr_bound) {
        *out = m_read(addr);
        return 0;
    }

    return 1; 
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
    addr_t curr_base = blocks[curr_addr].base;
    addr_t curr_size = blocks[curr_addr].size;

    if (addr >= curr_base && addr < curr_base + curr_size) {
        m_write(addr, val);
        return 0;
    }

    return 1; 
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
    curr_pid = process.pid; // Actualiza el pid del proceso actual.
    curr_addr = procs_addr[process.pid]; // Actualiza la dirección actual.
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
    addr_t addr = procs_addr[process.pid]; // Obtiene la dirección del proceso.
    m_unset_owner(blocks[addr].base, blocks[addr].bound);
    blocks[addr].in_execution = 0;
    blocks[addr].pid = NO_ONWER;
    blocks[addr].size = 0;
    blocks[addr].heap = blocks[addr].base;
    blocks[addr].stack = blocks[addr].bound;
}
