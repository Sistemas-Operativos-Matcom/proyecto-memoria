#include "bnb_manager.h"
#include "structs.h"
#include "memory.h"
#include "stdio.h"

process_t actual_proc;

List bnb;

LFList frees;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
    bnb = Init();
    frees = Init_LF(m_size());
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
    bandb *proc = Find(actual_proc, &bnb);
    out->size = size;
    out->addr = proc->heap - proc->base;
    proc->heap += size;
    return proc->heap >= proc->stack ? 0 : 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
    return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
    bandb *proc = Find(actual_proc, &bnb);
    proc->stack--;
    m_write(proc->stack, val);
    out->addr = proc->stack - proc->base;
    out->size = 1;
    return proc->stack == proc->heap ? 0 : 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
    bandb *proc = Find(actual_proc, &bnb);
    *out = m_read(proc->stack);
    proc->stack++;
    return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
    bandb *proc = Find(actual_proc, &bnb);
    *out = m_read(proc->base + addr);
    return addr > proc->bounds ? 0 : 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
    bandb *proc = Find(actual_proc, &bnb);
    m_write(proc->base + addr, val);
    proc->heap = addr > proc->heap ? addr : proc->heap;
    return addr > proc->bounds ? 0 : 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
    if (Exist(process, &bnb) == 0) // verifico primero que el proceso nuevo ya tenga un espacio en memoria reservado
    {
        LFelement *filled = Fill_Space(64, &frees);                                                                              // si no es asi le reservo 64 bytes de memoria
        bandb new = {process, filled->start, filled->size, filled->start + process.program->size, filled->start + filled->size}; // creo un objeto bandb donde alammceno el proceso con su información de su memoria reservada
        Push(new, &bnb);                                                                                                         // lo agrego a la lista de bandb
        set_curr_owner(process.pid);
        m_set_owner(filled->start, filled->start + filled->size); // pongo al proceso como el owner de ese espacio de memoria
        actual_proc = process;
    }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
    bandb removed = *Find(process, &bnb);
    Remove(removed, &bnb);
    Free_Space(removed.base, removed.bounds, &frees);
    m_unset_owner(removed.base, removed.base + removed.bounds);
}
