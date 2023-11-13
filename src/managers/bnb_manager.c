#include "bnb_manager.h"

#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>

static addr_t addr_now;
static addr_t *addrProc;
static int pid;
static memory_global_t *mem;
#define boxSize 1000


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
    if (addrProc != NULL) {
        free(addrProc);
        addrProc = NULL;
    }
    if (mem != NULL) {
        free(mem);
        mem = NULL;
    }
    size_t count = m_size()/boxSize;

    addrProc = (size_t *)malloc(count * sizeof(size_t));
    mem = (memory_global_t *)malloc(count * sizeof(memory_global_t));
    addr_now = 0;

    for (size_t i = 0,start = 0; i < count; i++ , start += boxSize) {
        memory_global_t* aux = &mem[i];
        aux -> start = start + 1;
        aux -> end = start + boxSize - 1;
        aux -> stack = start + boxSize - 1;
        aux -> heap = start + 1;
        aux -> size = 0;
        aux -> active = 0;
        aux -> use = NO_ONWER;
    }

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    for(size_t i = 0; i < m_size()/boxSize; i++){
        if(!mem[i].active)
        {
            size_t aux = i * boxSize;
            m_set_owner(aux + 1, aux + boxSize - 1);
            mem[i].active = 1;
            mem[i].use = pid;
            mem[i].size = size;
            addrProc[pid] = i;
            addr_now = i;
            out->addr = aux + 1;
            out->size = size;
            return 0;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
    if(ptr.addr >= mem[addr_now].start && ptr.size + ptr.addr < mem[addr_now].end)
    {
        m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
        mem[addr_now].size -= ptr.size;
        return 0;
    }
    return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
    if(mem[addr_now].stack - 1 <= mem[addr_now].heap)
        return 1;
    m_write(mem[addr_now].stack, val);
    mem[addr_now].stack--;
    out->addr = mem[addr_now].stack;
    return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
    if(mem[addr_now].stack + 1 >= mem[addr_now].end + mem[addr_now].start)
        return 1;
    *out = m_read(mem[addr_now].stack + 1);
    mem[addr_now].stack++;
    return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
    if(addr >= mem[addr_now].start && addr < mem[addr_now].end)
    {
        *out = m_read(addr);
        return 0;
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
    if(addr >= mem[addr_now].start && addr < mem[addr_now].start + mem[addr_now].size)
    {
        m_write(addr, val);
        return 0;
    }
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
    pid = process.pid;
    addr_now = addrProc[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
    m_unset_owner(mem[addr_now].start, mem[addr_now].end);
    mem[addrProc[process.pid]].stack = mem[addrProc[process.pid]].end;
    mem[addrProc[process.pid]].heap = mem[addrProc[process.pid]].start;
    mem[addrProc[process.pid]].active = 0;
    mem[addrProc[process.pid]].use = NO_ONWER;
    mem[addrProc[process.pid]].size = 0;
}
