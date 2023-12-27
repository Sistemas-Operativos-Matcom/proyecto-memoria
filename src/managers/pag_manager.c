#include "pag_manager.h"
#include "stdio.h"
#include "memory.h"
#include "data_structures.h"

static int pages_phyisical_owner[MAX_NUMBER_PAGES];
static int number_pages;

static int ccurrent_owner_pag;
static pcb my_arr_proc_pag[MAX_NUMBER_PROGRAMS];
static int n_active_programs_pag;

unsigned long translator_pag(unsigned long v_address, int *page_table)
{
    unsigned long bucket_index = v_address >> PAGE_SIZE;
    unsigned long offset = v_address % (1 << PAGE_SIZE);
    unsigned long p_address = page_table[bucket_index] * (1 << PAGE_SIZE) + offset;
    return p_address;
}

void set_owner_page(int index_page)
{
    unsigned long offset = (1 << PAGE_SIZE) * index_page;
    m_set_owner(offset, offset + (1 << PAGE_SIZE) - 1);
}

void unset_owner_page(int index_page)
{
    unsigned long offset = (1 << PAGE_SIZE) * index_page;
    m_unset_owner(offset, offset + (1 << PAGE_SIZE) - 1);
}

void pcb_paging_init(int *n_free, process_t process)
{
    for (int i = 0; i < N_PAGES_PER_PROCESS; i++)
    {
        pages_phyisical_owner[n_free[i]] = process.pid;
    }
    pcb_init(&my_arr_proc_pag[n_active_programs_pag], 0, (1 << PAGE_SIZE) * N_PAGES_PER_PROCESS - 1, process.program->size, process.pid);
    my_arr_proc_pag[n_active_programs_pag].page_table = n_free;
    n_active_programs_pag++;
    ccurrent_owner_pag = process.pid;
    set_curr_owner(process.pid);
    for (int i = 0; i < N_PAGES_PER_PROCESS; i++)
    {
        set_owner_page(n_free[i]);
        pages_phyisical_owner[n_free[i]] = process.pid;
    }
}

void m_pag_init(int argc, char **argv)
{
    size_t memory_size = m_size();
    ccurrent_owner_pag = -1;
    number_pages = memory_size >> PAGE_SIZE;
    for (int i = 0; i < number_pages; i++)
    {
        pages_phyisical_owner[i] = -1;
    }
    n_active_programs_pag = 0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (can_insert(my_arr_proc_pag[i].fl_heap, size))
            {
                unsigned long address_in_fl = request_space(my_arr_proc_pag[i].fl_heap, size);
                out->addr = address_in_fl + my_arr_proc_pag[i].heap_start;
                out->size = size;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (is_recoverable(my_arr_proc_pag[i].fl_heap, ptr.addr - my_arr_proc_pag[i].heap_start, ptr.size))
            {
                recover_space(my_arr_proc_pag[i].fl_heap, ptr.addr - my_arr_proc_pag[i].heap_start, ptr.size);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (my_arr_proc_pag[i].stack_pointer != my_arr_proc_pag[i].stack_end - 1)
            {
                m_write(translator_pag(my_arr_proc_pag[i].stack_pointer, my_arr_proc_pag[i].page_table), val);
                out->addr = my_arr_proc_pag[i].stack_pointer;
                out->size = 1;
                my_arr_proc_pag[i].stack_pointer--;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (my_arr_proc_pag[i].stack_pointer != my_arr_proc_pag[i].stack_start)
            {
                *out = m_read(translator_pag(my_arr_proc_pag[i].stack_pointer + 1, my_arr_proc_pag[i].page_table));
                my_arr_proc_pag[i].stack_pointer++;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Carga el valor de una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (is_occupied(my_arr_proc_pag[i].fl_heap, addr - my_arr_proc_pag[i].heap_start))
            {
                *out = m_read(translator_pag(addr, my_arr_proc_pag[i].page_table));
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == ccurrent_owner_pag)
        {
            if (is_occupied(my_arr_proc_pag[i].fl_heap, addr - my_arr_proc_pag[i].heap_start))
            {
                m_write(translator_pag(addr, my_arr_proc_pag[i].page_table), val);
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
    int found = 0;
    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == process.pid)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ccurrent_owner_pag = process.pid;
        set_curr_owner(ccurrent_owner_pag);
    }
    else
    {
        int *n_free = (int *)malloc(sizeof(int) * N_PAGES_PER_PROCESS);
        int n_found = 0;
        for (int i = 0; i < number_pages; i++)
        {
            if (pages_phyisical_owner[i] == -1)
            {
                n_free[n_found++] = i;
                if (n_found == N_PAGES_PER_PROCESS)
                {
                    break;
                }
            }
        }
        if (n_found == N_PAGES_PER_PROCESS)
        {
            pcb_paging_init(n_free, process);
            return;
        }
        else
        {
            ccurrent_owner_pag = NO_ONWER;
            set_curr_owner(ccurrent_owner_pag);
            return;
        }
    }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{

    for (int i = 0; i < n_active_programs_pag; i++)
    {
        if (my_arr_proc_pag[i].pid == process.pid)
        {
            pcb *to_delete = &my_arr_proc_pag[i];
            for (int j = 0; j < N_PAGES_PER_PROCESS; j++)
            {
                unset_owner_page(to_delete->page_table[j]);
                pages_phyisical_owner[to_delete->page_table[j]] = -1;
            }
            free(to_delete->page_table);
            free_list_free(to_delete->fl_heap);
            for (int j = i + 1; j < n_active_programs_pag; j++)
            {
                my_arr_proc_pag[j - 1] = my_arr_proc_pag[j];
            }
            n_active_programs_pag--;
            break;
        }
    }
    if (process.pid == ccurrent_owner_pag)
    {
        ccurrent_owner_pag = NO_ONWER;
        set_curr_owner(ccurrent_owner_pag);
    }
    return;
}