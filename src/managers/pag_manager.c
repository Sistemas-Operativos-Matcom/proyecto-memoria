#include "pag_manager.h"

#include "stdio.h"
#define current context[actual_pid]
#include "queue.h"

// para usar el codigo heap y stack los tratare todos en una matriz
// donde si hay un uno es heap, dos stack, tres codigo
// y la stack se ira llevando tambien en un queue

typedef struct Context {
  int pid;    
  int pages[4];
  int data[4][512];
  struct queue* stack;
} t_context;

t_context* context; // contexto de los procesos
int actual_pid = -1; // contexto actual
int page_size = 512; // tamano de pagina
int page_cant = 4; // cantidad de paginas por proceso
int page_frame_size = -1; // cantidad de paginas en la memoria real
int *page_frame; // el mapeo de el page frame de la memoria real a que proceso le pertenece

// busca si ese proceso ya existe y devuelve su posicion e el array de contextos
int search_context(int pid) {
  for (int i = 0; i < page_frame_size; i++) {
    if (context[i].pid == pid)
    return i;
  }
  return -1;
}

int empty_context() {
  for (int i = 0; i < page_frame_size; i++) {
    if (context[i].pid == -1){
      return i;
    }
  }
  return -1;
}
int empty_page_frame() {
  for (int i = 0; i < page_frame_size; i++) {
    if (page_frame[i] == -1)
    return i;
  }
  return -1;
}

// obtiene de la direccion local la real
int dir_page(int a) { 
  return a / 512;
}
// obtiene de la direccion local la real
int dir_local(int a) { 
  return a % 512;
}

int fill_code(int pos, int a, int b) {
  for (int i = a; i <= b; i++) {
    current.data[pos][i] = 3;
  }
}

int fill_heap(int a, int size) {
  // mapea a las direcciones locales y tomandolo como si fuera un array consecutivo escribe
  int ini_page = dir_page(a); 
  int ini_local = dir_local(a); 
  int take = 0;
  for (int i = ini_page; i < 4; i++) {
    int j = (i == ini_page ? ini_local : 0);

      // printf("%i %i \n", i, j);
    while (j < 512)
    {
      if (take == size) return 0;
      current.data[i][j] = 1;
      printf("%i %i \n", i, j);
      take++;
      j++;
    }
     
  }
  return 0;
}

// si un page esta libre devuelve 1 y sino 0
int is_free(int page) {
  for (int i = 0; i < 512; i++) {
    if (current.data[page][i] != 0) {
      printf("i %i", i);
      return 0;
    }
  }
  return 1;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  page_frame_size = m_size() / page_size;
  context = (t_context*)malloc(page_frame_size * sizeof(t_context));
  page_frame = (int*)malloc(page_frame_size * sizeof(int));
  for (size_t i = 0; i < page_frame_size; i++) {
    page_frame[i] = -1; // esta vacia la memoria
    context[i].pid = -1;  
    context[i].stack = new_queue(10000);
    for (size_t j = 0; j < 3; j++) {
      context[i].pages[j] = -1;
    }
  }

  enqueue(context[0].stack, 43);
  enqueue(context[0].stack, 45);
  printf("queue %i\n", front((context[0].stack)));
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int take = 0;
  int init = -1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 512; j++){
      // si necesito un nuevo page
      if (current.pages[i] == -1) {
        int frame = empty_page_frame();
        // busca si hay 
        if (frame != -1) {
          page_frame[frame] = current.pid;
          current.pages[i] = frame;
          m_set_owner(frame * 512, frame * 512 + 512 - 1);
        } else {
          return 1;
        }
      }
      if (current.data[i][j] == 0) {
        take++;
        if (init == -1) {
          init = (i + 1) * j;
        }
      } else {
        take = 0;
        init = -1;
      }
      if (take == size) { 
        out->addr = init; // guarda la direccion relativa a la memoria del proceso
        out->size = size;
        fill_heap(init, size);
        return 0;
      }
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 512; j++){
      // si necesito un nuevo page
      if (current.pages[i] == -1) {
        int frame = empty_page_frame();
        // busca si hay 
        if (frame != -1) {
          page_frame[frame] = current.pid;
          current.pages[i] = frame;
          m_set_owner(frame * 512, frame * 512 + 512 - 1);
        } else {
          return 1;
        }
      }
      if (current.data[i][j] == 0) {
        int p = i * 512 + j;
        int pos = current.pages[i] * 512 + j;
        printf("%i\n", p);
        m_write(pos, val);
        printf("size %i", size(context->stack));
        enqueue(context->stack, p);
        printf("front %i\n", front(context->stack));
        return 0;
      } 
    }
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  int addr = front(current.stack);
  dequeue(current.stack);
  
  int page = dir_page(addr);
  int local = dir_local(addr);
  current.data[page][local] = 0;
  if (is_free(page) == 1) {
    int page_real = current.pages[page];
    page_frame[page_real] = -1;
    current.pages[page] = -1;
    m_unset_owner((page_real+1) * 512, (page_real+1) * 512 + 512 - 1);
  }
  int p = current.pages[page] * 512 + local;
  printf("%i ", current.data[page][0]);
  *out = m_read(p);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  int page = dir_page(addr);
  int local = dir_local(addr);
  printf("%i ", current.pages[page] * 512 + local );
  if (current.pages[page] != -1) {
    if (current.data[page][local] == 1) {
      *out = m_read(current.pages[page] * 512 + local);
      return 0;
    }
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  int page = dir_page(addr);
  int local = dir_local(addr);
  // printf("%i ", current.data[page][local -1] );
  if (current.pages[page] != -1) {
    if (current.data[page][local] == 1) {
      m_write(current.pages[page] * 512 + local, val);
      return 0;
    }
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  int pos = search_context(process.pid);
  if (pos == -1) { // busquemos si el proceso es nuevo donde pueda caber
    int empty = empty_context(); // donde puede estar en el array de contexto
 
    if (empty != -1){
      actual_pid = empty;

      // ahora busquemos el primer page frame donde quepa
      current.pid = process.pid;
      int frame = empty_page_frame(); 
      if (frame != -1) { // si hay un page frame libre
        page_frame[frame] = current.pid;
        current.pages[0] = frame;
        // printf("%i %i\n", frame * 512, frame * 512 + 512 - 1);
        m_set_owner(frame * 512, frame * 512 + 512 - 1);

        // escribe el codigo en ese frame
        int code = process.program->size;
        int frames_code = code % 512;
        fill_code(0, 0, code);
        // for(int i = 0; i < 512; i ++ ) {
        //   printf("%i ", code);
        // }
        return;
      }
    }
    // si no cabe da error
    fprintf(stderr, "sin espacios disponibles\n");
    exit(1);
  }
  actual_pid = pos;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
