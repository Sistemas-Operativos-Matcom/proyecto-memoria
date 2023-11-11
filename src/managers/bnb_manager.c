#include "bnb_manager.h"
#include "stdio.h"
#include <stdlib.h>

#include "./data_structures.h"

free_list *fl_memory_bnb;
static pcb my_arr_bnb[MAX_NUMBER_PROGRAMS];
static int len_bnb = 0;
static int current_owner_bnb = -1;


void pcb_bb_init(pcb *process, unsigned long start, unsigned long end, unsigned long code_size, int pid)
{
    /*
    call this function passing address in physical memory and not viceversa.
    */
    process->pid = pid;
    process->code_start = start;
    process->code_end = start + code_size;
    process->heap_start = start + code_size + 1;
    process->heap_end = start + code_size + (end - start + 1 - code_size) / 2;
    process->stack_end = process->heap_end + 1;
    process->stack_start = end;
    process->stack_pointer = end;
    process->fl_heap = create_fl(process->heap_end - process->heap_start + 1);
}

void free_bnb_pcb(pcb *process)
{
    m_unset_owner(process->code_start, process->code_end);
    m_unset_owner(process->heap_start, process->heap_end);
    m_unset_owner(process->stack_end, process->stack_start);
    free_list_free(process->fl_heap);
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
    fl_memory_bnb = create_fl(m_size());
    len_bnb = 0;
    return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (can_insert(my_arr_bnb[i].fl_heap, size))
            {
                out->addr = request_space(my_arr_bnb[i].fl_heap, size);
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (is_recoverable(my_arr_bnb[i].fl_heap, ptr.addr, ptr.size))
            {
                recover_space(my_arr_bnb[i].fl_heap, ptr.addr, ptr.size);
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (my_arr_bnb[i].stack_pointer != my_arr_bnb[i].stack_end - 1)
            {
                m_write(my_arr_bnb[i].stack_pointer, val);
                out->addr = my_arr_bnb[i].stack_pointer;
                out->size = 1;
                my_arr_bnb[i].stack_pointer--;
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (my_arr_bnb[i].stack_pointer != my_arr_bnb[i].stack_start)
            {
                *out = m_read(my_arr_bnb[i].stack_pointer + 1);
                my_arr_bnb[i].stack_pointer++;
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (is_occupied(my_arr_bnb[i].fl_heap, addr))
            {
                *out = m_read(my_arr_bnb[i].heap_start + addr);
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == current_owner_bnb)
        {
            if (is_occupied(my_arr_bnb[i].fl_heap, addr))
            {
                m_write(my_arr_bnb[i].heap_start + addr, val);
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
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == process.pid)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        current_owner_bnb = process.pid;
        set_curr_owner(current_owner_bnb);
    }
    else
    {
        unsigned long size = process.program->size + PROGRAM_SIZE;
        if (can_insert(fl_memory_bnb, size))
        {
            current_owner_bnb = process.pid;
            set_curr_owner(process.pid);
            unsigned long addr = request_space(fl_memory_bnb, size);
            m_set_owner(addr, addr + size - 1);
            pcb_bb_init(&my_arr_bnb[len_bnb], addr, addr + size - 1, process.program->size, process.pid);
            len_bnb++;
        }
        else
        {
            current_owner_bnb = NO_ONWER;
            set_curr_owner(current_owner_bnb);
        }
    }
    return;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
    for (int i = 0; i < len_bnb; i++)
    {
        if (my_arr_bnb[i].pid == process.pid)
        {
            pcb *to_delete = &my_arr_bnb[i];
            recover_space(fl_memory_bnb, to_delete->code_start, to_delete->stack_end - to_delete->code_start + 1);
            free_bnb_pcb(to_delete);
            for (int j = i + 1; j < len_bnb; j++)
            {
                my_arr_bnb[j - 1] = my_arr_bnb[j];
            }
            len_bnb--;
            break;
        }
    }
    if (process.pid == current_owner_bnb)
    {
        current_owner_bnb = NO_ONWER;
        set_curr_owner(current_owner_bnb);
    }
    return;
}