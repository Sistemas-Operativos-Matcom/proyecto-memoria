#include "pag_manager.h"
#include "stdio.h"

#define MAX_PROGRAM_COUNT 20

//Devuelve los 5 bits mas significativos de 13 en decimal
#define VPN(dec) (dec & ~255) >> 8

//Devuelve los primeros 8 bits en decimal
#define OFFSET(dec) dec & 255

page_info_t page_table [32];
int Current_pid = -1;
int Dir = 0;


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  for (size_t i = 0; i < 32; i++)
  {
    page_table[i].pfn = -1;
    page_table[i].pte = -1;
    page_table[i].owner = -1;
    page_table[i].data = i*256;
    page_table[i].datainit = i*256;
    for (size_t i = 0; i < 256; i++)
    {
      page_table[i].owner_ptr_pg = -1;
    }
  }
  return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  if (Current_pid == -1){
    printf("Error en Malloc");
    return 1;
  }
  if (page_table[Dir].data + size >= page_table[Dir + 1].datainit){
    //Reservar lo que quepa en esta page y el resto
    //coger otra page y reservarlo en esa
  }
  for (size_t i = 0; i < size; i++)
  {
    page_table[Dir].owner_ptr_pg[i + page_table[Dir].data] = page_table[Dir].data;
  }

  out->addr = page_table[Dir].data;
  page_table[Dir].data += size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  int count = 0;
  int compare = ptr.addr;
  for (size_t i = ptr.addr; page_table[Dir].owner_ptr_pg[i] == compare; i++)
  {
    page_table[Dir].owner_ptr_pg[i] = -1;
    count++;
  }
  
  page_table[Dir].data = page_table[Dir].data - count;
  ptr.size = 0;
  ptr.addr = -1;
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  fprintf(stderr, "Not Implemented\n");
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  fprintf(stderr, "Not Implemented\n");
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  fprintf(stderr, "Not Implemented\n");
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  fprintf(stderr, "Not Implemented\n");
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  Current_pid = process.pid;
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].owner == Current_pid) return;
  }
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].owner == -1){
      page_table[i].owner = Current_pid;
      Dir = i;
      m_set_owner(i*256 + 1, (i+1)*256 - 1);
      return;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  fprintf(stderr, "Not Implemented\n");
  return;
}
