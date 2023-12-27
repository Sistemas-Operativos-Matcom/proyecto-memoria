#include "bnb_manager.h"
#include "stdio.h"
#include <stdlib.h>

#include "./data_structures.h"

static int ccurrent_owner_bnb;
static pcb my_arr_proc_bnb[MAX_NUMBER_PROGRAMS];
static int n_active_programs_bnb;
free_list *fl_memory_bnb;

void free_bnb_pcb(pcb *process)
{
    m_unset_owner(process->p_address, process->p_address + process->stack_start);
    free_list_free(process->fl_heap);
}

unsigned long translator_bb(unsigned long addr, pcb process)
{
    return addr + process.p_address;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
    fl_memory_bnb = create_fl(m_size());
    n_active_programs_bnb = 0;
    ccurrent_owner_bnb = -1;
    return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (can_insert(my_arr_proc_bnb[i].fl_heap, size))
            {
                out->addr = request_space(my_arr_proc_bnb[i].fl_heap, size);
                out->size = size;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (is_recoverable(my_arr_proc_bnb[i].fl_heap, ptr.addr  , ptr.size))
            {
                recover_space(my_arr_proc_bnb[i].fl_heap, ptr.addr, ptr.size);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (my_arr_proc_bnb[i].stack_pointer != my_arr_proc_bnb[i].stack_end - 1)
            {
                my_arr_proc_bnb[i].stack_pointer--;
                m_write( translator_bb( my_arr_proc_bnb[i].stack_pointer, my_arr_proc_bnb[i]) , val);
                out->addr = my_arr_proc_bnb[i].stack_pointer;
                out->size = 1;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (my_arr_proc_bnb[i].stack_pointer != my_arr_proc_bnb[i].stack_start+1)
            {
                *out = m_read(translator_bb( my_arr_proc_bnb[i].stack_pointer, my_arr_proc_bnb[i]));
                my_arr_proc_bnb[i].stack_pointer++;
                return 0;
                
            }
            break;
        }
    }
    return 1;
}

// Carga el valor en una dirección determinada.
int m_bnb_load(addr_t addr, byte *out)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (is_occupied(my_arr_proc_bnb[i].fl_heap, addr))
            {
                *out = m_read(translator_bb(my_arr_proc_bnb[i].heap_start + addr, my_arr_proc_bnb[i]));
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == ccurrent_owner_bnb)
        {
            if (is_occupied(my_arr_proc_bnb[i].fl_heap, addr))
            {
                m_write(translator_bb(my_arr_proc_bnb[i].heap_start + addr, my_arr_proc_bnb[i]), val);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
    int found = 0;
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == process.pid)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ccurrent_owner_bnb = process.pid;
        set_curr_owner(ccurrent_owner_bnb);
    }
    else
    {
        unsigned long size = process.program->size + PROGRAM_SIZE;
        if (can_insert(fl_memory_bnb, size))
        {
            ccurrent_owner_bnb = process.pid;
            unsigned long addr = request_space(fl_memory_bnb, size);
            pcb_init(
                &my_arr_proc_bnb[n_active_programs_bnb], 
                0, 
                size - 1, 
                process.program->size, 
                process.pid);
            my_arr_proc_bnb[n_active_programs_bnb].p_address = addr;
            set_curr_owner(process.pid);
            m_set_owner(addr, addr + size - 1);
            n_active_programs_bnb++;
        }
        else
        {
            ccurrent_owner_bnb = NO_ONWER;
            set_curr_owner(ccurrent_owner_bnb);
        }
    }
    return;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
    for (int i = 0; i < n_active_programs_bnb; i++)
    {
        if (my_arr_proc_bnb[i].pid == process.pid)
        {
            pcb *to_delete = &my_arr_proc_bnb[i];
            recover_space(fl_memory_bnb, to_delete->p_address, to_delete->stack_end+1);
            free_bnb_pcb(to_delete);
            for (int j = i + 1; j < n_active_programs_bnb; j++)
            {
                my_arr_proc_bnb[j - 1] = my_arr_proc_bnb[j];
            }
            n_active_programs_bnb--;
            break;
        }
    }
    if (process.pid == ccurrent_owner_bnb)
    {
        ccurrent_owner_bnb = NO_ONWER;
        set_curr_owner(ccurrent_owner_bnb);
    }
    return;
}