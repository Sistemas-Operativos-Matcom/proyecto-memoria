#include "bnb_manager.h"

#include "stdio.h"

static segment_t *segments;
static addr_t *processes_addres;

static addr_t current_segment;
static int current_pid;

#define segment_size 1024


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t segments_amount = m_size() / segment_size;


  //reiniciamos los valores de memoria virtual y procesos para el siguiente caso de prueba
  current_segment = 0;

  if(segments != NULL){{
    free(segments);
    segments = NULL;
  }}

  if(processes_addres =! NULL){
    free(processes_addres);
    processes_addres = NULL;
  }

  //inicializamos los valores por defecto
  segments = (segment_t *)malloc(sizeof(segment_t) * segments_amount);
  processes_addres = (addr_t *)malloc(sizeof(addr_t) * segment_size);


  for(size_t i = 0; i < segments_amount; i++){
    segment_t* this_segment = &segments[i];

    this_segment->base = i*segment_size + 1;
    this_segment->bound = i*segment_size + segment_size - 1;
    this_segment->heap_pointer = i*segment_size + 1;
    this_segment->stack_pointer = i*segment_size + segment_size - 1;

    this_segment->in_use = 0;
    this_segment->process_in_use = -1;
    this_segment->size = 0;  


  }

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //buscamos el primer segmento de memoria libre y reservamos en memoria el espacio correspondiente a ese segmento
  //seteamos los valores de size con los del metodo y definimos el proceso correspondiente
  //ademas al hacer malloc debemos llevar los datos del proceso y del segmento en cuestion
  for(size_t i = 0; i < m_size() / segment_size; i++){
    if(!segments[i].in_use){

      m_set_owner(i * segment_size + 1, i * segment_size + segment_size - 1);

      segments[i].in_use = 1;
      segments[i].size = size;
      segments[i].process_in_use = current_pid;

      processes_addres[current_pid] = i;
      current_segment = i;

      return 0;
    }
    return 1;
  }
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //comprobamos que el puntero sea valido en cuyo caso liberamos el correspondiente espacio en memoria
  //y modificamos el size del segmento acutal en correspondencia
  if(ptr.addr >= segments[current_segment].base && ptr.addr <= segments[current_segment].bound){
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);

    segments[current_segment].size = segments[current_segment].size - ptr.size;

    return 1;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //comprobamos si hay espacio en el stack
  if(segments[current_segment].stack_pointer - 1 <= segments[current_segment].heap_pointer){
    return 1;
  }

  //agregamos el valor a memoria en la direccion correspiente
  m_write(segments[current_segment].stack_pointer, val);

  //devolvemos el puntero al dato y modificamos el puntero hacia el stack del segmento actual
  out->addr = segments[current_segment].stack_pointer - 1;
  segments[current_segment].stack_pointer = segments[current_segment].stack_pointer - 1;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //comprobamos si hay algun elemento en la pila
  if(segments[current_segment].base + segments[current_segment].bound <= segments[current_segment].stack_pointer + 1){
    return 1;
  }

  //obtenemos el valor en memoria correspondiente al stack del segmento
  //actualizamos el stack
  *out = m_read(segments[current_segment].stack_pointer + 1);
  segments[current_segment].stack_pointer = segments[current_segment].stack_pointer + 1;
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  //comprobamos que la direccion dada pertenezca al segmento actual
  //almacenamos en out el valor que se encuentra en dicha direccion
  if(addr >= segments[current_segment].base && addr <= segments[current_segment].bound){
    *out = m_read(addr);
    return 0;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  if(addr >= segments[current_segment].base && addr <= segments[current_segment].bound){
    m_write(addr, val);
    return 0;
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  current_pid = process.pid;
  current_segment = processes_addres[process.pid];
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  //fprintf(stderr, "Not Implemented\n");
  //exit(1);

  size_t ending_process_segment = processes_addres[process.pid];
  
  //liberamos el espacio en memoria correspondiente al segmento al cual pertencia el proceso
  m_unset_owner(segments[ending_process_segment].base, segments[ending_process_segment].bound);

  //reiniciamos a los valores por defecto el segmento correspondiente
  segments[ending_process_segment].in_use = 0;
  segments[ending_process_segment].process_in_use = -1;
  segments[ending_process_segment].size = 0;
  segments[ending_process_segment].heap_pointer = segments[ending_process_segment].base;
  segments[ending_process_segment].stack_pointer = segments[ending_process_segment].bound;
  

}
