#include "seg_manager.h"

#include "./data_structures.h"

static int ccurrent_owner_seg;
static pcb my_arr_proc_seg[MAX_NUMBER_PROGRAMS];
static int n_active_programs_seg;
free_list *fl_memory_seg;

void pcb_seg_init(
    pcb *process, 
    unsigned long code_start, unsigned long code_size,
    unsigned long heap_start, unsigned long heap_size,
    unsigned long stack_start, unsigned long stack_size, 
    int pid)
{
    /*
    call this function passing address in physical memory and not viceversa.
    */
    process->pid = pid;
    process->code_start = code_start;
    process->code_end = code_start + code_size-1;
    process->heap_start = heap_start;
    process->heap_end = heap_start + heap_size - 1;
    process->stack_end = stack_start;
    process->stack_start = stack_start + stack_size -1;
    process->stack_pointer = stack_start + stack_size -1;
    process->fl_heap = create_fl(process->heap_end - process->heap_start + 1);
}

void free_seg_pcb(pcb *process)
{
    m_unset_owner(process->code_start, process->code_end);
    m_unset_owner(process->heap_start, process->heap_end);
    m_unset_owner(process->stack_end, process->stack_start);
    free_list_free(process->fl_heap);
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
    fl_memory_seg = create_fl(m_size());
    n_active_programs_seg = 0;
    ccurrent_owner_seg = -1;
    return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (can_insert(my_arr_proc_seg[i].fl_heap, size))
            {
                out->addr = request_space(my_arr_proc_seg[i].fl_heap, size);
                out->size = size;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (is_recoverable(my_arr_proc_seg[i].fl_heap, ptr.addr, ptr.size))
            {
                recover_space(my_arr_proc_seg[i].fl_heap, ptr.addr, ptr.size);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (my_arr_proc_seg[i].stack_pointer != my_arr_proc_seg[i].stack_end - 1)
            {
                m_write(my_arr_proc_seg[i].stack_pointer, val);
                out->addr = my_arr_proc_seg[i].stack_pointer;
                out->size = 1;
                my_arr_proc_seg[i].stack_pointer--;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (my_arr_proc_seg[i].stack_pointer != my_arr_proc_seg[i].stack_start)
            {
                *out = m_read(my_arr_proc_seg[i].stack_pointer + 1);
                my_arr_proc_seg[i].stack_pointer++;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (is_occupied(my_arr_proc_seg[i].fl_heap, addr))
            {
                *out = m_read(my_arr_proc_seg[i].heap_start + addr);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == ccurrent_owner_seg)
        {
            if (is_occupied(my_arr_proc_seg[i].fl_heap, addr))
            {
                m_write(my_arr_proc_seg[i].heap_start + addr, val);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
    int found = 0;
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == process.pid)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ccurrent_owner_seg = process.pid;
        set_curr_owner(ccurrent_owner_seg);
    }
    else
    {
        unsigned long code_size = process.program->size + 1;
        if (can_insert(fl_memory_seg, code_size))
        {
            unsigned long add_code = request_space(fl_memory_seg, code_size);
            unsigned long heap_size = (PROGRAM_SIZE - (code_size)) / 2;
            if (can_insert(fl_memory_seg, heap_size))
            {
                unsigned long add_heap = request_space(fl_memory_seg, heap_size);
                unsigned long stack_size = PROGRAM_SIZE - (code_size + heap_size);
                if (can_insert(fl_memory_seg, stack_size))
                {
                    unsigned long add_stack = request_space(fl_memory_seg, stack_size);
                    ccurrent_owner_seg = process.pid;
                    set_curr_owner(process.pid);
                    m_set_owner(add_code, add_code + process.program->size);
                    m_set_owner(add_heap, add_heap + heap_size - 1);
                    m_set_owner(add_stack, add_stack + stack_size - 1);
                    pcb_seg_init(&my_arr_proc_seg[n_active_programs_seg], add_code, code_size, add_heap, heap_size, add_stack, stack_size, process.pid);
                    n_active_programs_seg++;
                }
                else
                {
                    recover_space(fl_memory_seg, add_heap, heap_size);
                    recover_space(fl_memory_seg, add_code, code_size);
                }
            }
            else
            {
                recover_space(fl_memory_seg, add_code, code_size);
                return;
            }
        }
        else
        {
            ccurrent_owner_seg = NO_ONWER;
            set_curr_owner(ccurrent_owner_seg);
        }
    }
    return;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
    for (int i = 0; i < n_active_programs_seg; i++)
    {
        if (my_arr_proc_seg[i].pid == process.pid)
        {
            pcb *to_delete = &my_arr_proc_seg[i];
            recover_space(fl_memory_seg, to_delete->code_start, to_delete->code_end - to_delete->code_start + 1);
            recover_space(fl_memory_seg, to_delete->heap_start, to_delete->heap_end - to_delete->heap_start + 1);
            recover_space(fl_memory_seg, to_delete->stack_start, to_delete->stack_end - to_delete->stack_start + 1);
            free_seg_pcb(to_delete);
            for (int j = i + 1; j < n_active_programs_seg; j++)
            {
                my_arr_proc_seg[j - 1] = my_arr_proc_seg[j];
            }
            n_active_programs_seg--;
            break;
        }
    }
    if (process.pid == ccurrent_owner_seg)
    {
        ccurrent_owner_seg = NO_ONWER;
        set_curr_owner(ccurrent_owner_seg);
    }
    return;
}