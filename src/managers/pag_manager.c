#include <stdio.h>
#include "pag_manager.h"
#include "aux_structs/free_list.h"
#include "aux_structs/pag_process.h"

#define ALL_ONES64 ((size_t) 0xffffffffffffffff)

// Mantener estos valores en potencias de 2 para evitar errores!!!
static size_t MAX_PROC_COUNT = 8;
static size_t const PAGE_SIZE = 64;
static size_t const MAX_PAGES_IN_PROC = 32;
static size_t const MAX_ADDR = PAGE_SIZE * MAX_PAGES_IN_PROC;

static size_t MEM_SIZE;
static size_t VPN_MASK; // VPN = virtual page number
static size_t VPN_SHIFT; 

static free_list_t* FREE_LIST; // Esta free_list tiene las paginas, no memoria per-se
static pag_process_t* procs;
static size_t curr_proc =  0;
static int last_proc = 0;
static bool is_init = false;
bool init_again = false;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  MEM_SIZE = m_size();
  
  size_t offset_bits = _log2(PAGE_SIZE);
  
  VPN_MASK = ALL_ONES64 ^ ((1 << offset_bits) - 1);
  VPN_SHIFT = offset_bits;

  if (is_init) {
    fl_reset_list(FREE_LIST, MEM_SIZE / PAGE_SIZE);
    free(procs);
    init_again = true;
  }
  else {
    FREE_LIST = (free_list_t*) malloc(sizeof(free_list_t));
    fl_init_list(FREE_LIST, MEM_SIZE / PAGE_SIZE);
  }

  procs = (pag_process_t*) malloc(MAX_PROC_COUNT * sizeof(pag_process_t));
  last_proc = 0;
  curr_proc = 0;

  for (size_t i = 0; i < MAX_PROC_COUNT; i++) {
    procs[i].pid = -1;
  }

  is_init = true;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  // Reservar un conjunto de paginas contiguas suficientemente grande
  size = size / PAGE_SIZE + (size_t) (size % PAGE_SIZE ? 1 : 0);
  
  // Encontrar una posicion donde guardar los segmentos reservados
  size_t pos = find_space(&procs[curr_proc], size);
  
  // Si no existe dicha posicion, error
  if (pos == ~0ul) {
    return MEM_FAIL;
  }

  size_t i = pos;

  // Por cada pagina necesaria, se reserva
  while (size != 0) {
    size_t addr;

    if (fl_get_memory(FREE_LIST, 1, &addr))
      return MEM_FAIL;
    
    m_set_owner(addr * PAGE_SIZE, addr * PAGE_SIZE + 1 * PAGE_SIZE);
    
    procs[curr_proc].vpn[i] = (addr + i);
    procs[curr_proc].valid[i] = true;

    i ++;
    size --;
  }

  // Guardar en out la direccion virtual donde estara la memoria
  out->addr = pos * PAGE_SIZE;
  out->size = size * PAGE_SIZE;

  // Retornar con exito
  return MEM_SUCCESS;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  // Obtener el numero de pagina virtual
  size_t first_page = (ptr.addr & VPN_MASK) >> VPN_SHIFT;
  // Obtener cuantas paginas hay que liberar
  size_t page_count = ptr.size / PAGE_SIZE + (size_t) (ptr.size % PAGE_SIZE ? 1 : 0);

  // Liberar la memoria
  if (fl_free_memory(FREE_LIST, page_count, procs[curr_proc].vpn[first_page])) 
    return MEM_FAIL;

  m_unset_owner(first_page * PAGE_SIZE, first_page * PAGE_SIZE + page_count * PAGE_SIZE);

  // Marcar la memoria como no valida
  for (size_t i = first_page; page_count; i++, page_count--) {
    procs[curr_proc].valid[i] = false;
  }

  return MEM_SUCCESS;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  // Encontrar el stack point fisico
  size_t sp = procs[curr_proc].stack_point;
         sp = sp >= MAX_ADDR ? MAX_ADDR - 1 : sp;
  size_t page = (sp & VPN_MASK) >> VPN_SHIFT;
  size_t offset = (sp & ~VPN_MASK) - 1;

  // page = page > MAX_ADDR/PAGE_SIZE - 1 ? MAX_ADDR - 1 : page;

  // Si esta pagina no es valida, se reserva una y se asigna aqui
  if (!procs[curr_proc].valid[page]) {
    size_t tmp;
    
    if (fl_get_memory(FREE_LIST, 1, &tmp))
      return MEM_FAIL;
    
    m_set_owner(tmp * PAGE_SIZE, (tmp + 1) * PAGE_SIZE);
    
    procs[curr_proc].vpn[page] = tmp;
    procs[curr_proc].valid[page] = true;
  }

  // Calcular la direccion fisica
  size_t addr = procs[curr_proc].vpn[page] * PAGE_SIZE + offset;

  // Guardar el valor ahi
  m_write(addr, val);
  
  // Calcular el nuevo stack point
  sp = (page << VPN_SHIFT) | offset;
  
  // Devolver el valor en el puntero
  out->addr = sp;
  out->size = 1;

  // Actualizar el stack point
  procs[curr_proc].stack_point = sp;

  // Retornar con exito
  return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  // Encontrar el stack point fisico
  size_t sp = procs[curr_proc].stack_point;
  size_t page = (sp & VPN_MASK) >> VPN_SHIFT;
  size_t offset = sp & ~VPN_MASK;

  // Chequear si la accion es valida
  if (sp >= MAX_ADDR || !procs[curr_proc].valid[page])
    return MEM_FAIL;

  // Calcular la direccion fisica
  size_t addr = procs[curr_proc].vpn[page] * PAGE_SIZE + offset;

  // Guardar en out el resultado
  *out = m_read(addr);

  // Actualizar el stack point
  procs[curr_proc].stack_point = ++ sp;

  // Si se sale de una pagina
  if (page != (sp & VPN_MASK) >> VPN_SHIFT) {
    addr = procs[curr_proc].vpn[page];
    
    // Liberar esa pagina
    if (fl_free_memory(FREE_LIST, 1, addr))
      return MEM_FAIL;
    
    m_unset_owner(addr * PAGE_SIZE, (addr + 1) * PAGE_SIZE);
  }

  //Retornar con exito
  return MEM_SUCCESS;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  // Obtener el numero de pagina virtual
  size_t page = (addr & VPN_MASK) >> VPN_SHIFT;
  // Obtener el offset
  size_t offset = addr & ~VPN_MASK;

  // Obtener la direccion fisica
  size_t _addr = procs[curr_proc].vpn[page] * PAGE_SIZE + offset;
  
  // Chequear que sea valida la operacion
  if (_addr > MAX_ADDR || !procs[curr_proc].valid[page])
    return MEM_FAIL;
  
  // Retornar con exito
  *out = m_read(_addr);
  return MEM_SUCCESS;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  // Obtener el numero de pagina virtual
  size_t page = (addr & VPN_MASK) >> VPN_SHIFT;
  // Obtener el offset
  size_t offset = ~VPN_MASK & (size_t) addr;

  // Obtener la direccion fisica
  size_t _addr = procs[curr_proc].vpn[page] * PAGE_SIZE + offset;

  // Chequear que sea valida la operacion
  if (_addr > MAX_ADDR || !procs[curr_proc].valid[page])
    return MEM_FAIL;

  m_write(_addr, val);
   
  // Retornar con exito
  return MEM_SUCCESS;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  int last_free = -1;
  // Buscar un espacio para el proceso, si ya esta solo se devuelve
  for (int i = 0; i < last_proc; i++) {
    if (procs[i].pid == process.pid) {
      curr_proc = i;
      return;
    }
    if (!procs[i].is_active) {
      last_free = i;
    }
  }

  // En caso de que el proceso sea nuevo, se añade en la primera
  // posicion donde no halla ninguno
  int pos = last_free == -1 ? last_proc++ : last_free;
  
  pag_process_init(&procs[pos], process.pid, MAX_PAGES_IN_PROC);
  
  curr_proc = pos;  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  // Encontrar el proceso
  for (int i = 0; i < last_proc; i++) {
    if (procs[i].pid == process.pid) {
      // Marcarlo como no activo
      procs[i].is_active = false;
      
      // Liberar la memoria que ocupa
      for (int l = 0, r; (size_t) l < procs[i].total_mem; l++) {
        if (procs[i].valid[l]) {
          r = l;
          while (procs[i].valid[++r]);
          fl_free_memory(FREE_LIST, r - l, l);
          l = r;
        }
      }

      free(procs[i].vpn);
      free(procs[i].valid);
    }
  }
}
