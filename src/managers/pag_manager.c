#include "pag_manager.h"
#include "stdio.h"

#define MAX_PROGRAM_COUNT 20

//Devuelve los 5 bits mas significativos de 13 en decimal
#define VPN(dec) (dec & ~255) >> 8

//Devuelve los primeros 8 bits en decimal
#define OFFSET(dec) (dec & 255)

page_info_t page_table [32];
int Current_pid = -1;
int Dir = 0;
int owner_ptr_pg[100000];


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  for (size_t i = 0; i < 32; i++)
  {
    page_table[i].owner = -1;
    page_table[i].order_owner = 0;
    page_table[i].data = i*256;
    page_table[i].datainit = i*256;
    page_table[i].owner_ptr_pg = owner_ptr_pg;
    for (size_t j = 0; j < 256; j++)
    {
      page_table[i].owner_ptr_pg[j] = -1;
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
  if (page_table[Dir].data + size > page_table[Dir].datainit + 255){
    //Reservar lo que quepa en esta page y el resto
    //coger otra page y reservarlo en esa
    int size_p1 = (page_table[Dir].datainit + 255) - page_table[Dir].data;
    int size_p2 = size - size_p1;
    for (int i = 0; i < size_p1; i++)
    {
      page_table[Dir].owner_ptr_pg[i + page_table[Dir].data] = page_table[Dir].data;
    }
    page_table[Dir].data += size_p1;
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].owner == -1){
        page_table[i].owner = Current_pid;
        page_table[i].order_owner = page_table[Dir].order_owner + 1;
        Dir = i;
        m_set_owner(i*256 + 1, (i+1)*256 - 1);
        for (int i = 0; i < size_p2; i++)
        {
          page_table[Dir].owner_ptr_pg[i + page_table[Dir].data] = page_table[Dir].data;
        }
        out->addr = (((Dir) << 8) | (page_table[Dir].data));
        page_table[Dir].data += size_p2;
        return 0;
      } 
    }
    printf("Error en Malloc, paginas llenas");
    return 1;
  }
  for (size_t i = 0; i < size; i++)
  {
    page_table[Dir].owner_ptr_pg[i + page_table[Dir].data] = page_table[Dir].data;
  }
  
  out->addr = (((Dir) << 8) | (page_table[Dir].data));
  page_table[Dir].data = page_table[Dir].data + size;
  
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
  if (Dir == -1){
    fprintf(stderr, "Error en push\n");
    return 1;
  } 
  if (page_table[Dir].data == page_table[Dir].datainit + 255){
    //Crear una nueva page y poner el valor en ella
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].owner == -1){
        page_table[i].owner = Current_pid;
        page_table[i].order_owner = page_table[Dir].order_owner + 1;
        Dir = i;
        m_set_owner(i*256 + 1, (i+1)*256 - 1);

        m_write(page_table[Dir].data, val);
        page_table[Dir].data ++;
        out->addr = (((Dir) << 8) | (page_table[Dir].data));
        return 0;
      }
    }
    fprintf(stderr, "Error en push, paginas llenas\n");
    return 1;
  }
  m_write(page_table[Dir].data, val);
  page_table[Dir].data ++;
  out->addr = (((Dir) << 8) | (page_table[Dir].data));
  return 0;
}


// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if (Dir == -1){
    fprintf(stderr, "Error en pop\n");
    return 1;
  } 
  if (page_table[Dir].data == page_table[Dir].datainit){
    //Ver si hay una page anterior, eliminar esta y buscar en ella
    //sino error
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].owner == page_table[Dir].owner && page_table[i].order_owner < page_table[Dir].order_owner){
        Dir = i;
        page_table[Dir].data --;
        *out = m_read(page_table[Dir].data);
        return 0;
      }
    }
    fprintf(stderr, "Error en pop, no hay mas paginas\n");
    return 1;
  }
  page_table[Dir].data --;
  *out = m_read(page_table[Dir].data);
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  if (page_table[VPN(addr)].data < (OFFSET(addr))){
    fprintf(stderr, "Error en store\n");
    return 1;
  }
   *out = m_read(page_table[VPN(addr)].datainit + (OFFSET(addr)));
   return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  if (page_table[VPN(addr)].data < OFFSET(addr))
  {
    fprintf(stderr, "Error en store\n");
    return 1;
  }
  m_write(page_table[VPN(addr)].datainit + OFFSET(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  Current_pid = process.pid;
  Dir = 0;
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].owner == Current_pid) return;
    Dir++;
  }
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].owner == -1)
    {
      page_table[i].owner = Current_pid;
      Dir = i;
      m_set_owner(i*256, (i+1)*256 - 1);
      return;
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  int pos = -1;
  int aux = 0;
  while(1)
  {
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].owner == process.pid){
        pos = i;
        aux = 1;
        break;
      }
    }
    if (aux) break;
    if (pos == -1) break;
    m_unset_owner(page_table[pos].datainit, page_table[pos].datainit + 255);
  }
}
