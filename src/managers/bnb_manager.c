#include "bnb_manager.h"

#include "stdio.h"
#define current context[actual]

typedef struct Context {
  int pid;
  int base;
  int code;
  int p_stack;  
  int heap[1024];
} t_context;

int h = 1024;
t_context* context; 
int actual = -1;

int m = -1;
int cant = -1;


void fill(int a, int b) {
  for (int i = a; i < b; i++) {
    current.heap[i] = 1;
  }  
}
void clean(int a, int b) {
  for (int i = a; i <= b; i++) {
    current.heap[i] = 0;
  }  
}
// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) { 
  // establezco la cantidad de procesos que puedo tener como en cuantos puedo dividir en la memeoria en trozos de 1024
  m = m_size();
  cant = m / h;

  // construyo los contextos como array donde la posicion es el pid
  context = (t_context*)malloc(cant * sizeof(t_context));
  for (size_t i = 0; i < cant; i++) {
    context[i].pid = -1;
    context[i].base = 0 + h * i;
    context[i].p_stack = h - 1;
    for (size_t j = 0; j < h; j++) {
      context[i].heap[j] = 0;
    }
    
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  int pos = 0; 
  int stack = current.p_stack;
  int code = current.code;
  int* heap = current.heap;
  // printf("%i",stack);
  while (code + pos < stack) { // pues no pueden solaparse el heap y el stack
    // buscare el primer bloque de ceros
    if (heap[pos] == 0) {
      int init = pos; 
      while (heap[pos] == 0) {
        pos++;
        if (pos - init == size && pos < stack) {
          fill(init, pos);
          // printf("malloc %d %d", init, pos);
          out->addr = init + code; // guarda la direccion relativa a la memoria del proceso
          out->size = size;   
          return 0;
        }
      }
    }
    pos++;
  }
  // for (int i = 0; i < 1024; i ++) {
  //   printf("%i ", head[i]);
  // }
  // no se puede reservar memoria
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int a = ptr.addr;
  int b = ptr.addr + ptr.size - 1;
  clean(a, b);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) { 
  if (current.p_stack >= h) {
    return 1;
  }
  // guarda y obten
  m_write(current.p_stack + current.base, val);
  out = m_read(current.p_stack + current.base);
 
  current.p_stack--; 
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  if (current.p_stack >= h - 1) { // chequea que la pila no este vacia
    return 1;
  }

  // printf("pop %i\n", current.p_stack + 1 + current.base);
  *out = m_read(current.p_stack + 1 + current.base);
  current.p_stack++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if (current.heap[addr - current.code] == 1) { // la posicion del heap es addr - el codigo
    *out = m_read(current.base + addr);
    return 0;
  } 
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  // si es una direccion valida del heap pide esa dirrecion
    // printf("%i \n", current.base + addr);
  if (current.heap[addr - current.code] == 1) { // la posicion del heap es addr - el codigo
    m_write(current.base + addr, val);
    return 0;
  }
  // printf("load %i ", addr);
  // for (int i = 0; i < 8 ; i++ ){
  //   printf("%i ", current->heap[i]);
  // }
  // printf("\n");
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  int pid = process.pid;
  // si ya existe lo cambio a ese
  for (int i = 0; i < cant; i++) {
    if (context[i].pid == pid) {
      actual = i;
      return;
    }
  }
  // si no existe establezco ese proceso como actual y lo actualizo
  for (int i = 0; i < cant; i++) {
    if (context[i].pid == -1) {
      actual = i;
      context[i].code = process.program->size;
      context[i].pid = pid;
      m_set_owner(current.base, current.base + h - 1);
      return;
    }
  } 
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  int pid = process.pid;
  for (int i = 0; i < cant; i++) {
    context[i].pid = context[i].pid == pid ? -1 : context[i].pid;
  }
} 

void freeContext() {
  free(context);
}