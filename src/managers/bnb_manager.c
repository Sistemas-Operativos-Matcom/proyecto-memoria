#include "bnb_manager.h"
#include "BitMap.h"
#include "BitMap_Proces.h"
#include "stdio.h"

static MemoryManager* MEMORY_MANAGER;
static ProcessManager* PROCESS_MANAGER;
static FILE* file;
static int actualProcess;


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  PROCESS_MANAGER = initializeProcessManager(m_size(), 512);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  out->addr = ReserveMemory(MEMORY_MANAGER, size);
  out->size = size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  freeMemory(MEMORY_MANAGER, ptr.addr);
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  size_t addr = PushMemory(MEMORY_MANAGER);
  m_write(PROCESS_MANAGER->process[actualProcess].heap + addr, val);
  out->addr = addr;
  out->size = val;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  *out = m_read(PROCESS_MANAGER->process[actualProcess].heap + MEMORY_MANAGER->Stack);
  // printf("\n%ld\n",MEMORY_MANAGER->Stack+1);
  // printf("\n%ld\n",MEMORY_MANAGER->MemorySize-1);
  // printf("\n%d\n",m_read(PROCESS_MANAGER->process[actualProcess].heap + MEMORY_MANAGER->Stack));
  // printf("%d",MEMORY_MANAGER->memory[MEMORY_MANAGER->Stack]);
  MEMORY_MANAGER->memory[MEMORY_MANAGER->Stack] = 0;
  MEMORY_MANAGER->Stack+=1;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  // printf("\n\nSe intenta cargar de la memoria fisica: %ld \n", PROCESS_MANAGER->process[actualProcess].heap + addr);
  *out = m_read(PROCESS_MANAGER->process[actualProcess].heap + addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  // printf("%ld     %ld   ", PROCESS_MANAGER->process[actualProcess].heap, addr);
  // printf("\n\nSe escribio en la direccion fisica: %ld el valor %i\n", PROCESS_MANAGER->process[actualProcess].heap + addr, val);
  m_write(PROCESS_MANAGER->process[actualProcess].heap + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  // printf("Cambiamos de proceso\n");
  printf("\n----------------------------------%i-----------------------------------------\n", actualProcess);
  actualProcess = NewProcess(PROCESS_MANAGER, process);
  MEMORY_MANAGER = PROCESS_MANAGER->process[actualProcess].memory;
  m_set_owner(actualProcess*512, ((actualProcess+1)*512)-1);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  // printf("\nTERMINOOOOOooooooooooooooooo\n");
  int i = freeProcess(PROCESS_MANAGER, process);
  m_unset_owner(i*512, (i+1)*512);
}
