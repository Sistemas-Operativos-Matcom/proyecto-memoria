#include "pag_manager.h"
#include "stdio.h"
#include "../pag_process.h"

#define current_process pag_process[current_position]

int current_pid = -1;
int current_position = -1;

pag_process_t pag_process[32];
int find_page()
{
  for (size_t i = 0; i < 32; i++)
  {
    if (pag_process[i].pid == -1)
    {
      return i;
    }
  }
  return -1;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  for (size_t i = 0; i < 32; i++)
  {
    pag_process[i].pid = -1;
    pag_process[i].page_start = i * 256;
    pag_process[i].page_ip = i * 256;
    pag_process[i].process_part = 0;
    for (size_t j = 0; j < 256; j++)
    {
      pag_process[i].heap[j] = 0;
    }
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  if(current_pid==-1)return 1;
  if (current_process.page_ip + size <= current_process.page_start + 255)
  {
    for (size_t i = 0; i < size; i++)
    {
      current_process.heap[current_process.page_ip + i] = 1;
    }
    current_process.page_ip += size;
    out->addr = (current_position << 8 |current_process.page_ip ); //! revisar
    current_process.page_ip += size;
    return 0;
  }
  else
  {
    int first = (current_process.page_start + 255) - current_process.page_ip;
    int second = size - first;
    for (int i = 0; i < first; i++)
    {
      current_process.heap[current_process.page_ip + i] = 1;
    }
    current_process.page_ip += first;
    int new_page = find_page();
    if (new_page != -1)
    {
      pag_process[new_page].pid = current_pid;
      pag_process[new_page].process_part = current_process.process_part + 1;
      m_set_owner(pag_process[new_page].page_start +1, (pag_process[new_page].page_start + 255)-1);
      for (int i = 0; i < second; i++)
      {
        pag_process[new_page].heap[i + pag_process[new_page].page_ip] = 1;
      }
      current_position = new_page;
      out->addr = (current_process.page_ip | current_position << 8);
      current_process.page_ip += second;
      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  if (ptr.addr > current_process.page_ip)
  {
    return 1;
  }
  for (size_t i = ptr.addr; i < ptr.addr + ptr.size; i++)
  {
    current_process.heap[i] = 0;
  }
  current_process.page_ip -= ptr.size;
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{
  if (current_process.page_ip < current_process.page_start + 255)
  {
    m_write(current_process.page_ip, val);
    current_process.page_ip += 1;
    out->addr = (current_process.page_ip | current_position << 8);
    return 0;
  }
  else
  {
    int page = find_page();
    if (page != -1)
    {
      pag_process[page].pid = current_pid;
      pag_process[page].process_part = current_process.process_part + 1;
      m_set_owner(pag_process[page].page_start, pag_process[page].page_start + 255);
      m_write(pag_process[page].page_ip, val);
      current_position = page;
      current_process.page_ip += 1;
      out->addr = (current_process.page_ip | current_position << 8);
      return 0;
    }
    return 1;
  }
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  if (current_process.page_ip != current_process.page_start)
  {
    *out = m_read(current_process.page_ip-1);
    current_process.page_ip --;
    return 0;
  }
  
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  if (pag_process[((addr & ~255)>>8)].page_ip< (addr & 255)) 
  {
    return 1;
  }
  else
  {
    *out = m_read(pag_process[((addr & ~255)>>8)].page_start + (addr & 255));
    return 0;
  }
  
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{
  if (pag_process[((addr & ~255)>>8)].page_ip< (addr & 255)) 
  {
    return 1;
  }
  else
  {
    m_write(pag_process[((addr & ~255)>>8)].page_start + (addr & 255),val);
    return 0;
  }
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  current_pid  = process.pid;
  for (size_t i = 0; i < 32; i++)
  {
    if(pag_process[i].pid == current_pid)
    {
      current_position= i;
      return;
    }
  }
  current_position = find_page();
  if(current_position != -1)
  {
    pag_process[current_position].pid= current_pid;
    m_set_owner(current_process.page_start,current_process.page_start + 255);
    return;
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  for (size_t i = 0; i < 32; i++)
  {
    if(pag_process[i].pid == process.pid)
    {
      m_unset_owner(pag_process[i].page_start,pag_process[i].page_start+255);
      return;
    }
  }
}

