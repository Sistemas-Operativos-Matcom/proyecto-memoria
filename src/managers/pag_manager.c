#include "pag_manager.h"
#include "stdio.h"

// Estructura que representa el estado actual de un proceso
typedef struct curr_process {
    int in_execution;       // Indica si el proceso está en ejecución
    int pid;                // ID del proceso
    size_t *page_table;     // Tabla de páginas del proceso
    size_t heap;            // Puntero al inicio del heap
    size_t stack;           // Puntero al inicio del stack
} curr_process_t;

// Variables globales
static curr_process_t *processes;   // Arreglo de estructuras de procesos
static int *pages;                  // Arreglo que indica la ocupación de páginas
static int *index;                  // Índices de cada marco de página
static int curr_pid;                // ID del proceso actual
static int curr_index;              // Índice del proceso actual
static size_t num_pfs;              // Número de marcos de página

#define pag_size 256                // Tamaño de página en bytes

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
    // Liberar la memoria existente si es necesario
    free(pages);
    free(processes);
    free(index);

    // Calcular el número de marcos de página necesarios
    num_pfs = m_size() / pag_size;

    // Asignar nueva memoria
    processes = (curr_process_t *)malloc(num_pfs * sizeof(curr_process_t));
    index = (int *)malloc(num_pfs * sizeof(int));
    pages = (int *)malloc(num_pfs * sizeof(int));

    // Inicializar procesos y marcos de página
    for (size_t i = 0; i < num_pfs; i++) {
        curr_process_t *current_process = &processes[i]; 
        // Inicializa las variables para cada proceso
        current_process->in_execution = 0;
        current_process->page_table = (size_t *)malloc(4 * sizeof(size_t));

        for (size_t j = 0; j < 4; j++) {
            current_process->page_table[j] = -1;
        }

        current_process->pid = -1;
        current_process->heap = 0;
        current_process->stack = 4 * pag_size;

        // Marca el pf como no usado
        pages[i] = -1;
        index[i] = i;
    }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
    int tmp = 1;

    for (size_t i = 0; i < num_pfs; i++) {
        if (pages[i] == -1) {
            tmp = 0;
            out->addr = (i * pag_size);
            out->size = 1;
            pages[i] = curr_pid;
            processes[curr_index].page_table[0] = i;
            m_set_owner(i * pag_size, (i + 1) * pag_size - 1);
            break;
        }
    }

    return tmp;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
    size_t curr_pf = (size_t)(ptr.addr / pag_size);
    size_t end_pf = (size_t)((ptr.addr + ptr.size) / pag_size);
    int tmp = 0;

    if (end_pf >= num_pfs || ptr.size > processes[curr_index].heap) {
        return 1; // Error: intento de liberar memoria fuera de los límites.
    }

    for (size_t i = curr_pf; i < end_pf; i++) {
        if (pages[i] != curr_pid) {
            tmp = 1;
            return tmp; // Error: intento de liberar memoria no asignada al proceso actual.
        }
    }

    processes[curr_index].heap -= ptr.size;

    for (size_t i = 0; i < 4; i++) {
        size_t pf = processes[curr_index].page_table[i];

        if (pf > curr_pf && pf <= end_pf) {
            m_unset_owner(pf * pag_size, (pf + 1) * pag_size - 1);
            processes[curr_pf].page_table[i] = -1;
        }
    }

    return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
    size_t size_stack = (4 * pag_size) - processes[curr_index].stack;
    size_t page;

    if (processes[curr_index].heap + 1 == processes[curr_index].stack) {
        return 1; // Error: stack lleno, no se puede agregar más elementos.
    }

    // Agrega una nueva pagina a la memoria del programa actual
    if (size_stack % pag_size == 0) {
        for (size_t i = 0; i < num_pfs; i++) {
            if (pages[i] == -1) {
                pages[i] = curr_pid;
                page = 4 - (size_t)(size_stack / pag_size) - 1;
                processes[curr_index].page_table[page] = i;
                m_set_owner(i * pag_size, (i + 1) * pag_size - 1);
                break;
            }
        }
    }

    // Agrega un nuevo elemento al stack
    processes[curr_index].stack -= 1;
    size_stack += 1;
    page = 4 - (size_t)(size_stack / pag_size) - 1;
    size_t pf = processes[curr_index].page_table[page];
    size_t addr = (pf * pag_size) + (size_stack % pag_size);

    m_write(addr, val);

    out->addr = processes[curr_index].stack;

    return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
    if (processes[curr_index].stack == (4 * pag_size)) {
        return 1; // Error: stack vacío, no se puede quitar más elementos.
    }

    // Encuentra la direccion fisica
    size_t size_stack = (4 * pag_size) - processes[curr_index].stack;
    size_t page = 4 - (size_t)(size_stack / pag_size) - 1;
    size_t pf = processes[curr_index].page_table[page];
    size_t addr = (pf * pag_size) + (size_stack % pag_size);

    *out = m_read(addr);

    processes[curr_index].stack += 1;

    // En caso de terminar con el pf actual lo libera
    if (size_stack % pag_size == 0) {
        pages[pf] = -1;
        processes[curr_index].page_table[page] = -1;
        m_unset_owner(pf * pag_size, (pf + 1) * pag_size - 1);
    }

    return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
    size_t actual_page = (size_t)(addr / pag_size);

    if (pages[curr_index] == curr_pid) {
        *out = m_read(addr);

        return 0;
    }

    return 1; 
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
    size_t actual_page = (size_t)(addr / pag_size);

    if (pages[actual_page] == curr_pid) {
        m_write(addr, val);

        return 0;
    }

    return 1; 
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
    curr_pid = process.pid;

    for (size_t i = 0; i < num_pfs; i++) {
        if (pages[i] == process.pid) {
            curr_index = i;
            return;
        }
    }

    for (size_t i = 0; i < num_pfs; i++) {
        if (!processes[i].in_execution) {
            processes[i].in_execution = 1;
            processes[i].pid = process.pid;
            pages[i] = process.pid;
            curr_index = i;
            break;
        }
    }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
    for (size_t i = 0; i < num_pfs; i++) {
        if (processes[i].pid == process.pid) {
            processes[i].in_execution = 0;
            int pf;

            for (size_t j = 0; j < 4; j++) {
                pf = processes[i].page_table[j];

                if (pf != -1) {
                    pages[pf] = 0;
                }

                processes[i].page_table[j] = -1;
                m_unset_owner(i * pag_size, (i + 1) * pag_size - 1);
            }

            processes[i].pid = -1;
            processes[i].heap = 0;
            processes[i].stack = (4 * pag_size);
        }
    }
}
