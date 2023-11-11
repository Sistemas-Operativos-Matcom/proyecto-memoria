#include "pag_manager.h"
#include "BitMap.h"
#include "BitMap_Proces.h"
#include "stdio.h"
#include "BitMap_Proces_Pages.h"

static MemoryManager* MEMORY_MANAGER;
static ProcessPageManager* PROCESS_PAGE_MANAGER;
static FILE* file;
static int actualProcess;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  PROCESS_PAGE_MANAGER = initializeProcessPageManager(m_size(), 256);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  out->addr = ReservePageMemory(PROCESS_PAGE_MANAGER->process[actualProcess], size, PROCESS_PAGE_MANAGER->table, m_size()/256, 1);
  // printf("%iMMM",PROCESS_PAGE_MANAGER->process[actualProcess].Heap->start->Node);
  // printf("\n%ldAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n", out->addr);
  // printf("\nOK\n");
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  freeMemoryPage(PROCESS_PAGE_MANAGER->process[actualProcess].Heap, ptr.addr);
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  out->addr = ReservePageMemory(PROCESS_PAGE_MANAGER->process[actualProcess], val, PROCESS_PAGE_MANAGER->table, m_size()/256, 0);
  out->size = val;
  // printf("\nNode1 %i address1: %d\n", PROCESS_PAGE_MANAGER->process[actualProcess].Stack->start->Node, out->addr);
  out->addr = realaddrs(PROCESS_PAGE_MANAGER->process[actualProcess].Stack, out->addr);
  // printf("\naddrs: %ld, val: %d\n", out->addr, val);
  // printf("\nrealaddress: %li de %d\n", realaddrs(PROCESS_PAGE_MANAGER->process[actualProcess].Stack, out->addr), val);
  m_write(out->addr, val);
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  *out = m_read(realaddrs(PROCESS_PAGE_MANAGER->process[actualProcess].Stack, PROCESS_PAGE_MANAGER->process[actualProcess].Stack->start->virtualMemory->Stack));
  PROCESS_PAGE_MANAGER->process[actualProcess].Stack->start->virtualMemory->memory[PROCESS_PAGE_MANAGER->process[actualProcess].Stack->start->virtualMemory->Stack] = 0;
  PROCESS_PAGE_MANAGER->process[actualProcess].Stack->start->virtualMemory->Stack -= 1;
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  *out = m_read(realaddrs(PROCESS_PAGE_MANAGER->process[actualProcess].Heap, addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  size_t realaddr = realaddrs(PROCESS_PAGE_MANAGER->process[actualProcess].Heap, addr);
  printf("\nrealaddrs: %ld\n", realaddr);
  m_write(realaddr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  actualProcess = NewProcessPage(PROCESS_PAGE_MANAGER, process);
  printf("\n----------------------------------%i-----------------------------------------\n", actualProcess);
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  freeProcessPage(PROCESS_PAGE_MANAGER , process);
}
