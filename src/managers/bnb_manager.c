#include <stdio.h>
#include "bnb_manager.h"
#include "structs.h"
#include "memory.h"

process_t actual_proc;

List bnb;

LFList frees;

FILE *debug_bnb;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
    debug_bnb = fopen("./Debug/Debug.txt", "w");

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
    bandb *proc = Find(actual_proc, &bnb);
    byte from_addr = Search_addr(ptr.addr, &proc->mask_addr);
    byte to_addr = from_addr + ptr.size;
    for (size_t i = to_addr; i < proc->heap; i++)
    {
        m_write(from_addr, m_read(to_addr));
        from_addr++;
    }
    for (size_t i = 0; i < proc->mask_addr.length; i++)
    {
        if (proc->mask_addr.start[i].addr_v >= ptr.addr)
        {
            proc->mask_addr.start[i].addr_r -= ptr.size;
        }
    }
    proc->heap -= ptr.size;
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
    byte address = Search_addr(addr, &proc->mask_addr);
    if (address == 0)
        return 0;
    *out = m_read(address);
    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
    bandb *proc = Find(actual_proc, &bnb);
    Add_Mask(addr, proc->heap, &proc->mask_addr);
    m_write(proc->heap, val);
    proc->heap++;
    return proc->heap == proc->stack ? 0 : 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
    if (Exist(process, &bnb) == 0) // verifico primero que el proceso nuevo ya tenga un espacio en memoria reservado
    {
        LFelement *filled = Fill_Space(128, &frees);

        mask newmask = Init_Mask();                                                                                                       // si no es asi le reservo 64 bytes de memoria
        bandb new = {process, filled->start, filled->size, filled->start + process.program->size, newmask, filled->start + filled->size}; // creo un objeto bandb donde alammceno el proceso con su información de su memoria reservada
        Push(new, &bnb);                                                                                                                  // lo agrego a la lista de bandb
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
