#include "bnb_manager.h"
#include "../memory.h"
#include "stdio.h"
// #include "./aux_cont/free_list.c"


// START estructura free_list para gestionar el espacio libre en la memoria
// struct nodo de la free_list
struct free_list_n{
  int base;
  int bound;
  struct free_list_n *next_node;
};

// Struct free_list
struct free_list{
  int total_m;
  int nodec;
  struct free_list_n *node;
};
// FIN

// START estructura process_m para guardar los datos necesarios para gestionar
// la memoria de un proceso
// Estruct para guardar el bnb de un proc
struct bnb{
  int base;
  int bound;
};

// Struct para guardar los pointers de un proceso
struct pointers{
  int code_end; // puntero del final del codigo de el proc
  int heap; // puntero de el Heap
  int stack; // puntero de el stack
};

// struct para representar la mem de un proceso
struct process_m{
  int pid;
  struct bnb bnb;
  struct pointers pointers;
};
// FIN


// INSTANSIACION
int curr_pid; // current pid pid del proc que se esta ejecutando
int vms = 256; // virtual memoria size
int vmi;

// Instaciacion de las variables globales
struct process_m *process_list; // lista de los procesos
int pli = 0; // index para insertar nuevos procesos
struct free_list free_list;


// Asignar memoria
struct bnb assign_bnb_m(struct free_list_n node){
  if(node.bound>vms){//Si este nodo tiene suficiente espacio reservar memoria aqui
    struct bnb ret = {node.base,node.base+vms};
    m_set_owner(node.base, node.base+vms);
    node.base = node.base+vms;
    node.bound = node.bound - node.base;
    return ret;
  }
  if(node.next_node == &node){
    printf("\n No encontro memoria libre(bnb) \n");
  }
  struct free_list_n *nnp = node.next_node;
  return assign_bnb_m( *nnp );
}


// FUNC Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {

  // lista de todos los procesos
  /*
  process_list = (struct process_m *)malloc(sizeof(struct process_m)*argc);

  // lista de memoria libre
  free_list.total_m = m_size();
  free_list.nodec = 1;
  struct free_list_n node = {0,m_size(), &node};
  free_list.node = &node;*/


  fprintf(stderr, "Not Implemented\n");
  exit(1);

}

// FUNC Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  for(int i=0;i<pli;i++){
    if(process.pid == process_list[i].pid){
      curr_pid = process.pid;
      return;
    }
  }
  struct free_list_n *np= free_list.node;
  struct bnb np_bnb = assign_bnb_m(*np);
  // struct process_m new_proc = { process.pid};

  fprintf(stderr, "Not Implemented\n");
  exit(1);

}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}


