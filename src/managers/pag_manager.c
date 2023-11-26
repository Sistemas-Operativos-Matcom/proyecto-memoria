#include "pag_manager.h"

#include "stdio.h"
#include "string.h"

#define MaxProgCnt 32
#define Max_Pid 500000
#define HeapSize 4


static char* FMem;
int PagMemSlot[Max_Pid];
int PagProg_Size;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
    FMem=(char *)malloc(KB_SIZE(m_size()));
    int PagProg_Size=m_size()/MaxProgCnt;
    memset(PagMemSlot,-1,sizeof(PagMemSlot));
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
