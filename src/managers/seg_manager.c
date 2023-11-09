#include "seg_manager.h"

#include "stdio.h"

#define MAX_PROGRAM_COUNT 20

int currentPid = -1;
int owner_heap[100000];
int ip;
int sp;
int end;

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) {
  ip = 1;
  sp = m_size() - 1;
  end = sp + 1;
  for (int i = 0; i < end; i++)
  {
    owner_heap[i] = -1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {
  if (currentPid == -1) return 1;
  int siz = size;
  if (ip + siz >= sp)
  {
    printf("Accediste al espacio del stack");
    return 1;
  }

  for (int i = 0; i < siz; i++)
  {
    owner_heap[ip + i] = currentPid;
  }
  
  out->addr = ip;
  m_set_owner(ip, ip + siz);
  ip = ip + siz;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {
  ptr_t a = ptr;
  for (int i = 0; i < end; i++)
  {
    if (owner_heap[i] == currentPid){
      owner_heap[i] = -1;
    };
  }
  ptr.size = 0;
  ptr.addr = -1;
  return 0;
}



// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {
  if (sp - 1 == ip){
    fprintf(stderr, "Accediste al espacio del heap\n");
    return 1;
  }
  owner_heap[sp] = currentPid;
  m_set_owner(sp, sp);
  m_write(sp, val);
  out->addr = sp;
  sp--;
  return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {
  if (sp + 1 == end){
    fprintf(stderr, "No hay nada en el stack\n");
    return 1;
  }

  for (int i = ip; i < end; i++)
  {
    if (owner_heap[i] == currentPid){
      owner_heap[i] = -1;
      *out = m_read(i);
      m_unset_owner(i, i);
      return 0;
    }
  }

  fprintf(stderr, "No hay nada \n");
  return 1;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {
  int adr = addr;
  if (adr >= ip || owner_heap[adr] != currentPid){
    fprintf(stderr, "Accediste a espacio invalido\n");
    return 1;
  }

  *out = m_read(adr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {
  int adr = addr;
  if (adr >= ip || owner_heap[adr] != currentPid){
    fprintf(stderr, "Accediste a espacio invalido\n");
    return 1;
  }

  m_write(adr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {
  currentPid = process.pid;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  m_unset_owner(0, end - 1);
}
