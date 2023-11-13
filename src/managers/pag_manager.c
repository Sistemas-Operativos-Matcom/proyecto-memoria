#include "pag_manager.h"
#include <stdlib.h>
#include "stdio.h"

typedef struct memory_page{
    byte active;
    int use;
    size_t *page;
    size_t heap;
    size_t stack;
} memory_page_t;
static int pid;
static int index;
static size_t amount_pages;
static int *page_frame;
static int *mem;
static memory_page_t *mem_page;
#define PAGES 5

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
    if (mem != NULL)
    {
        free(mem);
        mem = NULL;
    }
    if (mem_page != NULL)
    {
        free(mem_page);
        mem_page = NULL;
    }
    if (page_frame != NULL)
    {
        free(page_frame);
        page_frame = NULL;
    }
    amount_pages = m_size()/200;
    mem_page = (memory_page_t *)malloc(amount_pages * sizeof(memory_page_t));
    page_frame = (int *)malloc(amount_pages * sizeof(amount_pages));
    mem = (int *)malloc(amount_pages * sizeof(int));
    for (size_t i = 0; i < amount_pages; i++)
    {
        mem_page[i].active = 0;
        mem_page[i].page = (size_t *)malloc(PAGES * sizeof(size_t));
        for (size_t j = 0; j < PAGES; j++)
        {
            mem_page[i].page[j] = -1;
        }
        mem_page[i].use = -1;
        mem_page[i].heap = 0;
        mem_page[i].stack = PAGES * 200;
        mem[i] = -1;
        page_frame[i] = i;
    }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
    int found = 1;
    for (size_t i = 0; i < amount_pages; i++)
    {
        if (mem[i] == -1)
        {
            found = 0;
            out->addr = (i * 200);
            out->size = 1;
            mem[i] = pid;
            mem_page[index].page[0] = i;
            m_set_owner(i * 200, (i + 1) * 200 - 1);
            break;
        }
    }
    return found;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
    int found = 0;
    if ((size_t)((ptr.addr + ptr.size) / 200) >= amount_pages || ptr.size > mem_page[index].heap)
        return 1;
    for (size_t i = (size_t)(ptr.addr / 200); i < (size_t)((ptr.addr + ptr.size) / 200); i++)
    {
        if (mem[i] != pid)
        {
            found = 1;
            break;
        }
    }
    if (found)
        return 1;
    mem_page[index].heap -= ptr.size;
    for (size_t i = 0; i < PAGES; i++)
    {
        if (mem_page[index].page[i] > (size_t)(ptr.addr / 200) && mem_page[index].page[i] <= (size_t)((ptr.addr + ptr.size) / 200))
        {
            m_unset_owner(mem_page[index].page[i] * 200, (mem_page[index].page[i] + 1) * 200 - 1);
            mem_page[(size_t)(ptr.addr / 200)].page[i] = -1;
        }
    }
    return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
    if (mem_page[index].heap + 1 == mem_page[index].stack)
        return 1;
    size_t size_now = PAGES * 200 - mem_page[index].stack;
    size_t page;
    if (size_now % 200 == 0)
    {
        for (size_t i = 0; i < amount_pages; i++)
        {
            if (mem[i] == -1)
            {
                mem[i] = pid;
                page = PAGES - (size_t)(size_now / 200) - 1;
                mem_page[index].page[page] = i;
                m_set_owner(i * 200, (i + 1) * 200 - 1);
                break;
            }
        }
    }
    mem_page[index].stack -= 1;
    size_now += 1;
    page = PAGES - (size_t)(size_now / 200) - 1;
    m_write((mem_page[index].page[page] * 200) + (size_now % 200), val);
    out->addr = mem_page[index].stack;
    return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
    if (mem_page[index].stack == PAGES * 200)
        return 1;
    size_t size_now = PAGES * 200 - mem_page[index].stack;
    size_t page = PAGES - (size_t)(size_now / 200) - 1;
    *out = m_read((mem_page[index].page[page] * 200) + (size_now % 200));
    mem_page[index].stack += 1;
    if (size_now % 200 == 0)
    {
        mem[mem_page[index].page[page]] = -1;
        mem_page[index].page[page] = -1;
        m_unset_owner(mem_page[index].page[page] * 200, (mem_page[index].page[page] + 1) * 200 - 1);
    }
    return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
    if (mem[index] == pid)
    {
        *out = m_read(addr);
        return 0;
    }
    return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
    if (mem[(size_t)(addr / 200)] == pid)
    {
        m_write(addr, val);
        return 0;
    }
    return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
    pid = process.pid;
    for (size_t i = 0; i < amount_pages; i++)
    {
        if (mem[i] == process.pid)
        {
            index = i;
            return;
        }
    }
    for (size_t i = 0; i < amount_pages; i++)
    {
        if (!mem_page[i].active)
        {
            mem_page[i].active = 1;
            mem_page[i].use = process.pid;
            mem[i] = process.pid;
            index = i;
            break;
        }
    }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
    for (size_t i = 0; i < amount_pages; i++)
    {
        if (mem_page[i].use == process.pid)
        {
            mem_page[i].active = 0;
            for (size_t j = 0; j < PAGES; j++)
            {
                if ((int)(mem_page[i].page[j]) != -1)
                    mem[(int)(mem_page[i].page[j])] = 0;
                mem_page[i].page[j] = -1;
                m_unset_owner(i * 200, (i + 1) * 200 - 1);
            }
            mem_page[i].use = -1;
            mem_page[i].heap = 0;
            mem_page[i].stack = PAGES * 200;
        }
    }
}