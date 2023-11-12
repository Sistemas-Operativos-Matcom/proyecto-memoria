#include "pag_manager.h"

#include "stdio.h"

#define MAX_PROGRAM_COUNT 20

//Devuelve los 5 bits mas significativos de 13 en decimal
#define VPN(dec) (dec & ~255) >> 8

//Devuelve los primeros 8 bits en decimal
#define OFFSET(dec) (dec & 255)

page_t page_table [32];
int current_pid = -1;
int page_index = 0;
int heap[100000];


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  for (size_t i = 0; i < 32; i++)
  {
    page_table[i].pid = -1;
    page_table[i].base = i*256;
    page_table[i].used = i*256;
    page_table[i].number = 0;
    for (size_t j = 0; j < 256; j++)
    {
      page_table[i].heap[j] = 0;
    }
  }
  return;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  if(current_pid == -1)
  {
    return 1;
  }
  if(page_table[page_index].base + size > page_table[page_index].used + 255) //si no cabe en una pagina busco otra pagina en la que reservar espacio
  {
    int size1 = (page_table[page_index].base + 255) - (page_table[page_index].used);
    int size2 = size - size1;
    for (int i = 1; i < size1; i++)
    {
      page_table[page_index].heap[i + page_table[page_index].used] = 1;
    }
    page_table[page_index].used += size1;
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].pid == -1){
        page_table[i].pid = current_pid;
        page_table[i].number = page_table[page_index].number + 1;
        page_index = i;
        m_set_owner(i*256 , (i+1)*256 - 1);
        for (int i = 0; i < size2; i++)
        {
          page_table[page_index].heap[i + page_table[page_index].used] = page_table[page_index].used;
        }
        out->addr = (((page_index) << 8) | (page_table[page_index].used));
        page_table[page_index].used += size2;
        return 0;
      } 
    }
    return 1; //No pude encontrar otra pagina libre
  }
  for (size_t i = 0; i < size; i++)
  {
    page_table[page_index].heap[i + page_table[page_index].used] = page_table[page_index].used;
  }
  
  out->addr = (((page_index) << 8) | (page_table[page_index].used));
  page_table[page_index].used = page_table[page_index].used + size;
  
  return 0;

}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  m_unset_owner(ptr.addr, ptr.addr + ptr.size);
  for (size_t i = ptr.addr; i < ptr.addr + ptr.size; i++)
  {
    page_table[page_index].heap[i] = 0;
    page_table[page_index].used --;
  }
  ptr.size = 0;
  ptr.addr = -1;
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if (page_table[page_index].used == page_table[page_index].base + 255){
    for (size_t i = 0; i < 32; i++)
    {
      if (page_table[i].pid == -1)
      {
        page_table[i].pid = current_pid;
        page_table[i].number = page_table[page_index].number + 1;
        m_set_owner(i*256 + 1, (i+1)*256 - 1);
        page_index = i;
        m_write(page_table[page_index].used, val);
        page_table[page_index].used ++;
        out->addr = (((page_index) << 8) | (page_table[page_index].used));
        return 0;
      }
    }
    return 1;
  }
  m_write(page_table[page_index].used, val);
  page_table[page_index].used ++;
  out->addr = (((page_index) << 8) | (page_table[page_index].used));
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  if (page_table[page_index].used != page_table[page_index].base)
  {
    page_table[page_index].used --;
    *out = m_read(page_table[page_index].used);
    return 0;
  }
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].pid == page_table[page_index].pid && page_table[i].number < page_table[page_index].number)
    {
      page_index = i;
      page_table[page_index].used --;
      *out = m_read(page_table[page_index].used);
      return 0;
    }
  }
  return 1;
  
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  if (page_table[VPN(addr)].used < (OFFSET(addr))){
    return 1;
  }
   *out = m_read(page_table[VPN(addr)].base + (OFFSET(addr)));
   return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  if (page_table[VPN(addr)].used < OFFSET(addr))
  {
    return 1;
  }
  m_write(page_table[VPN(addr)].base + OFFSET(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  current_pid = process.pid;
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].pid == current_pid)
    {
      page_index = i;
      return;
    }
  }
  for (size_t i = 0; i < 32; i++)
  {
    if (page_table[i].pid == -1)
    {
      page_table[i].pid = current_pid;
      page_index = i;
      m_set_owner(i*256, (i+1)*256 - 1);
      return;
    }
  }

  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  for (size_t i = 0; i < 32; i++)
  {
    if(page_table[i].pid == process.pid)
    {
      m_unset_owner(page_table[i].base, page_table[i].base + 255);
    }
  }
  
}
