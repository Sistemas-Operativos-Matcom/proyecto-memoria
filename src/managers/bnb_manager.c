#include "bnb_manager.h"

#include "stdio.h"

static int curr_pid;              // ID del proceso actual
static addr_t curr_addr;          // Dirección actual
static addr_t *procc_addr;       // direcciones de procesos
static Block_t *virt_mem;   // bloques de memoria

#define block_sz 1024
#define tam_Code 1
#define Kb(size) ((size) / block_sz)


void block_assign(Block_t *block, int start) {
    block->heap = start + tam_Code;
    block->stack = start + block_sz - 1;
    block->start = start + tam_Code;
    block->end = start + block_sz - 1;
    block->sz = 0;
    block->in_use = 0;
    block->user = NO_ONWER;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

    if (virt_mem != NULL) {
        free(virt_mem);
        virt_mem = NULL;
    }

    if (procc_addr != NULL) {
        free(procc_addr);
        procc_addr = NULL;
    }

    size_t blocks_cnt = Kb(m_size());

    virt_mem = (Block_t *)malloc(sizeof(Block_t) * blocks_cnt);
    procc_addr = (size_t *)malloc(sizeof(size_t) * blocks_cnt);
    curr_addr = 0;

    for (size_t i = 0, start = 0; i < blocks_cnt; i++, start += block_sz) {
        Block_t* curr_block = &virt_mem[i];
        block_assign(curr_block, start);
    }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out){

    for (size_t i = 0; i < Kb(m_size()); i++) {
        if (!virt_mem[i].in_use) {
            size_t aux = i * block_sz;
            m_set_owner(aux + tam_Code, aux + block_sz - 1);

            procc_addr[curr_pid] = i;
            curr_addr = i;
            virt_mem[i].in_use = 1;
            virt_mem[i].user = curr_pid;
            virt_mem[i].sz = size;


            out->addr = aux + tam_Code;
            out->size = 1;
            return 0;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

    size_t start = virt_mem[curr_addr].start;
    size_t end = virt_mem[curr_addr].end;

    if (ptr.addr >= start && ptr.addr + ptr.size < end) {
        m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
        virt_mem[curr_addr].sz -= ptr.size;
        return 0;
    }

    return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {

    if (virt_mem[curr_addr].stack - 1 <= virt_mem[curr_addr].heap) {
        return 1;
    }

    m_write(virt_mem[curr_addr].stack, val);
    virt_mem[curr_addr].stack--;
    out->addr = virt_mem[curr_addr].stack;
    return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {

    addr_t stack_top = virt_mem[curr_addr].stack + 1;
    addr_t block_start = virt_mem[curr_addr].start;
    addr_t block_end = virt_mem[curr_addr].end;

    if (block_start + block_end <= stack_top) {
        return 1;
    }

    *out = m_read(stack_top);
    virt_mem[curr_addr].stack++;
    return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {

    addr_t start = virt_mem[curr_addr].start;
    addr_t end = virt_mem[curr_addr].end;

  if (addr >= start && addr < end) {  // Comprueba si la dirección pertenece al bloque actual.
    *out = m_read(addr);  
    return 0;  
  }

    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  addr_t start_addr = virt_mem[curr_addr].start;
  addr_t curr_sz = virt_mem[curr_addr].sz;

  if (addr >= start_addr && addr < start_addr + curr_sz) {  // Comprueba si la dirección pertenece al bloque actual.
    m_write(addr, val);  
    return 0;  
  }

    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

    curr_pid = process.pid;
    curr_addr = procc_addr[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {

    addr_t addr = procc_addr[process.pid];
    m_unset_owner(virt_mem[addr].start, virt_mem[addr].end);
    virt_mem[addr].in_use = 0;
    virt_mem[addr].user = NO_ONWER;
    virt_mem[addr].sz = 0;
    virt_mem[addr].heap = virt_mem[addr].start;
    virt_mem[addr].stack = virt_mem[addr].end;
}
