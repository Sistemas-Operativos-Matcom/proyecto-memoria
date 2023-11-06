#include "bnb_manager.h"
#include "stdio.h"

#define MAX_PROGRAM_COUNT 20

vm_t memories [MAX_PROGRAM_COUNT];
int current_pid = -1;
int dir = -1;
int owner_ptr_heap[100000];

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  addr_t base = m_size() / MAX_PROGRAM_COUNT;
  for (size_t i = 0; i < MAX_PROGRAM_COUNT; i++)
  {
    memories[i].base = i * base;
    memories[i].ip = 1;
    memories[i].owner_ptr_heap = owner_ptr_heap;
    memories[i].fake_ip = 1;
    memories[i].bounds = memories[i].base + base - 1;
    memories[i].sp = base - 2;
    memories[i].pid = -1;
    for (size_t j = 0; j < base; j++)
    {
      memories[i].owner_ptr_heap[j] = -1;
    }
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  //Se hizo malloc sin cambiar de proceso
  if (dir == MAX_PROGRAM_COUNT) {
     fprintf(stderr, "Se hizo malloc sin cambiar de proceso\n");
    return 1;
  }

  //Compruebo que la nueva posicion del instruction pointer no sobrepase la del stack pointer
  if (memories[dir].ip + size >= memories[dir].sp) {
    printf("Accediste al espacio del stack");
    return 1;
  } 

  for (size_t j = memories[dir].ip; j < memories[dir].ip + size; j++)
  {
    memories[dir].owner_ptr_heap[j] = memories[dir].ip;
  }

  out->addr = memories[dir].ip;
  memories[dir].ip += size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int count = 0;
  int compare = ptr.addr;
  for (size_t j = ptr.addr; memories[dir].owner_ptr_heap[j] == compare; j++)
  {
    memories[dir].owner_ptr_heap[j] = -1;
    count++;
  }

  memories[dir].ip = memories[dir].ip - count;
  ptr.size = 0;
  ptr.addr = -1;
  return 0;
} 

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  if (dir == -1 || memories[dir].sp - 1 <= memories[dir].ip) {
    fprintf(stderr, "Accediste al espacio del heap\n");
    return 1;
  } 

  m_write(memories[dir].sp + memories[dir].base, val);
  memories[dir].sp --;
  out->addr = memories[dir].sp;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  if (memories[dir].base + memories[dir].sp + 1 == memories[dir].bounds) {
    fprintf(stderr, "No hay nada en el stack\n");
    return 1;
  } 

  memories[dir].sp ++;
  *out = m_read(memories[dir].sp + memories[dir].base);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if (addr >= memories[dir].ip || addr < memories[dir].fake_ip) {
    fprintf(stderr, "Accediste a espacio invalido\n");
    return 1;
  } 

  *out = m_read(addr + memories[dir].base);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  if (addr >= memories[dir].ip || addr < memories[dir].fake_ip) {
    fprintf(stderr, "Accediste a espacio invalido\n");
    return 1;
  } 

  m_write(addr + memories[dir].base, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  current_pid = process.pid;
  dir = find_pid(process.pid);

  if (dir == -1) exit(1);
  if (dir < MAX_PROGRAM_COUNT) return;

  dir = dir - MAX_PROGRAM_COUNT;
  m_set_owner(memories[dir].base, memories[dir].bounds);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  int pos = find_pid(process.pid);

  if (pos >= MAX_PROGRAM_COUNT || pos == -1) exit(1);

  m_unset_owner(memories[pos].base, memories[pos].bounds);
}

int find_pid(int pid){
  for (size_t i = 0; i < MAX_PROGRAM_COUNT; i++)
  {
    if (memories[i].pid == pid) {
      return i;
    }
  }

  for (size_t i = 0; i < MAX_PROGRAM_COUNT; i++)
  {
    if (memories[i].pid == -1) {
      memories[i].pid = pid;
      return i + MAX_PROGRAM_COUNT;
    }
  }
  
  return -1;
}