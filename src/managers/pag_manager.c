#include "pag_manager.h"

#include "stdio.h"


typedef struct page
{
  int vpfn;
  int val;
  int *free_list;
  // manejo del stack parche
  int stack_order;
}page_t;
typedef struct owner
{
  int pid;
  int stack_counter;
  page_t *space;
}owner_t;

int page_size;
int capacity_pag;
int curr_pid_pag;


owner_t *owner_pag;
// memoria fisica
int *page_frame;

int add_pf(){
  for (int i = 0; i < capacity_pag; i++)
  {
    if(page_frame[i] == 0)
    {
      m_set_owner(i*32, i*32 + 31);
      // reservar page_frame
      page_frame[i] = 1;
      return i;
    }
  }
  
  return -1;
}

void free_page_frame(int index_vpfn){
  page_frame[index_vpfn] = 0;
  m_unset_owner(index_vpfn * 32, index_vpfn*32 + 31);
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  page_size = 32;
  capacity_pag = m_size()/page_size;
  

  curr_pid_pag = -1;
  page_frame = (int *)malloc(capacity_pag * sizeof(int));
  for (int i = 0; i < capacity_pag; i++)
  {
    page_frame[i] = 0;
  }
  
  owner_pag = (owner_t *)malloc(capacity_pag * sizeof(owner_t));

  for (int i = 0; i < capacity_pag; i++)
  {
    owner_pag[i].pid = -1;
    owner_pag[i].stack_counter = -1;
  }
     
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }

  // buscar heap
  for (int i = 0; i < capacity_pag; i++)
  {
    // buscar en la free_list
    if(owner_pag[index_p].space[i].val == 0){      
      int count = 0;
      for(int j = 0; j < 32; j++){
    
    if(owner_pag[index_p].space[i].free_list[j] == 0){
      count ++;
      int val = 1;
      while(j+val < 32 && owner_pag[index_p].space[i].free_list[j + val] == 0){
        val ++;
        count ++;
      }
      if(count >= (int)size){
        // va
        out->addr = (size_t)i*32 + j;

        // rellenar 1
        for(int r = i; r < count; r ++){
          owner_pag[index_p].space[i].free_list[j] = 1;
        }
        return 0;
      }else{
        i = i + count -1;
        count = 0;
      }
    }
    }
  }  
  }
  // no hay pag con heap
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[index_p].space[i].val == -1){
      owner_pag[index_p].space[i].val = 0;
      int pf = add_pf();
      if (pf == -1)
      {
        return 1;
      }
      
      owner_pag[index_p].space[i].vpfn = pf;
      // inicializar free_list
      owner_pag[index_p].space[i].free_list = (int *)malloc(32 * sizeof(int));
      for (int fp = 0; fp < 32; fp++)
      {
        owner_pag[index_p].space[i].free_list[fp] = 0;
      }
      // ocupar size espacios en free_list
      out->addr = (size_t)i * 32;
      for (int p = 0; p < (int)size; p++)
      {
        owner_pag[index_p].space[i].free_list[p] = 1; 
      }
      return 0;    
      
    }
  }
  return 1;  
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {

  // hallar direccion de la memoria virtual
  int vpn = ptr.addr/32;
  int offset = ptr.addr % 32;
  // buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }
  if(owner_pag[index_p].space[vpn].val != 0){
    return 1;
  }
  // si la direccion en la free_list del heap ya ha sido 
  // liberada 
  if(owner_pag[index_p].space[vpn].val == 0 && owner_pag[index_p].space[vpn].free_list[offset] == 0){
    return 1;
  }
  // pag del heap, actualizar free_list
  for (int i = offset; i < offset + (int)ptr.size; i++)
  {
    owner_pag[index_p].space[vpn].free_list[i] = 0;
  }
  // verificar si la pag del heap quedo vacia
  int empty = -1;
  for (int i = 0; i < 32; i++)
  {
    if(owner_pag[index_p].space[vpn].free_list[i] == 1){
      empty = 0;
      break;
    }
  }
  if(empty == -1){
    owner_pag[index_p].space[vpn].val = -1;
    free_page_frame(owner_pag[index_p].space[vpn].vpfn);
  }


  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }

  // buscar en el space una pag de stack
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[index_p].space[i].val == 1){
      int stack_pointer = -1;
      // buscar en free_list
      for (int sp = 0; sp < 32; sp++)
      {
        if(owner_pag[index_p].space[i].free_list[sp] == 1){
          stack_pointer = sp;
          break;
        }
      }
      // pag de stack vacia
      if(stack_pointer == -1){
        owner_pag[index_p].space[i].free_list[31] = 1;
        int addr_vpfn = owner_pag[index_p].space[i].vpfn * 32;
        m_write(addr_vpfn + 31,val);
        
        out->addr = i*32 + 31;
        return 0;
      }
      // pag de stack llena
      if(stack_pointer == 0){
        
        // buscar pag libre en la memoria virtual
        for (int va = 0; va < capacity_pag; va++)
        {
          if(owner_pag[index_p].space[va].val == -1){
            owner_pag[index_p].space[va].val = 1;

            owner_pag[index_p].stack_counter ++;
            owner_pag[index_p].space[va].stack_order = owner_pag[index_p].stack_counter;


            int pf = add_pf();
            if (pf == -1)
            {
              return 1;
            }

            owner_pag[index_p].space[va].vpfn = pf;
            // inicializar el stack
            owner_pag[index_p].space[va].free_list[31] = 1;
            int addr_vpfn = owner_pag[index_p].space[va].vpfn * 32;
            m_write(addr_vpfn + 31,val);
            out->addr = va*32 + 31;


            return 0;
          }          
        }        
      }
      // pag de stack tiene espacio
      stack_pointer --;
      owner_pag[index_p].space[i].free_list[stack_pointer] = 1;
      int addr_vpfn = owner_pag[index_p].space[i].vpfn * 32;
      m_write(addr_vpfn + stack_pointer,val);
      out->addr = i*32 + stack_pointer;
      return 0;      
    }
  }
  // no hay pag de stack en space
  // buscar pag libre en la memoria virtual

  for (int va = 0; va < capacity_pag; va++)
  {
    if(owner_pag[index_p].space[va].val == -1){
      owner_pag[index_p].space[va].val = 1;

      owner_pag[index_p].stack_counter ++;
      owner_pag[index_p].space[va].stack_order = owner_pag[index_p].stack_counter;


      int pf = add_pf();
      if (pf == -1)
      {
        return 1;
      }

      owner_pag[index_p].space[va].vpfn = pf;

      // inicializar el free_list para el stack
      owner_pag[index_p].space[va].free_list =  (int *)malloc(capacity_pag * sizeof(int));
      for (int d = 0; d < 32; d++)
      {
        owner_pag[index_p].space[va].free_list[d] = 0;
      }
      
      owner_pag[index_p].space[va].free_list[31] = 1;
      
      int addr_vpfn = owner_pag[index_p].space[va].vpfn * 32;
      m_write(addr_vpfn + 31,val);
      out->addr = va*32 + 31;
      return 0;
    }          
  }    
  
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
   //buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }

  // buscar la pag del stack mas reciente  
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[index_p].space[i].stack_order == owner_pag[index_p].stack_counter){      
      // buscar ultimo 1
      int stack_pointer = -1;
      for (int sp = 0; sp < 32; sp++)
      {
        if (owner_pag[index_p].space[i].free_list[sp] == 1)
        {
          stack_pointer = sp;
          break;
        }
        
      }
      // 
      if (stack_pointer == 31)
      {
        // obtener el valor en el stack_pointer
        int addr = owner_pag[index_p].space[i].vpfn*32 + stack_pointer;
        *out = m_read(addr);

      
        // eliminar esta pag
        owner_pag[index_p].space[i].val = -1;
        // actualizar stack_counter del proceso
        owner_pag[index_p].stack_counter --;
        
        free_page_frame(owner_pag[index_p].space[i].vpfn);  

        return 0;       
      }else{
        int addr = owner_pag[index_p].space[i].vpfn*32 + stack_pointer;
        
        *out = m_read(addr);

        // actualizar free_list
        owner_pag[index_p].space[i].free_list[stack_pointer] = 0;
        
        return 0;
      }      
    }    
  }

  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }
  int index_vpn = addr/32;
  int offset = addr%32; 
  
  if(owner_pag[index_p].space[index_vpn].val != 0)
  {
    return 1;
  }

  int addr_vpfn = owner_pag[index_p].space[index_vpn].vpfn * 32;
  *out = m_read(addr_vpfn + offset);

  return 0;  
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  int index_vpn = addr/32;
  int offset = addr%32;
  //buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == curr_pid_pag){
      index_p = i;
      break;      
    }
  }
  if(owner_pag[index_p].space[index_vpn].val != 0)
  {
    return 1;
  }
  int addr_vpfn = owner_pag[index_p].space[index_vpn].vpfn * 32;
  m_write(addr_vpfn + offset,val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  curr_pid_pag = process.pid;
  
  for (int i = 0; i < capacity_pag; i++)
  {
    if (owner_pag[i].pid == curr_pid_pag)
    {      
     return;
    }    
  }
  // nuevo proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == -1){
      index_p = i;
      break;      
    }
  }
  // inicializar el espacio de direcciones para el nuevo proceso
  owner_pag[index_p].pid = curr_pid_pag;
  owner_pag[index_p].space = (page_t *)malloc(capacity_pag * sizeof(page_t));

  for (int i = 0; i < capacity_pag; i++)
  {
    owner_pag[index_p].space[i].val = -1;
    owner_pag[index_p].space[i].vpfn = -1;
    owner_pag[index_p].space[i].stack_order = -1;                
  }
  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {

    //buscar proceso
  int index_p = -1;
  for (int i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[i].pid == process.pid){
      index_p = i;
      break;      
    }
  }

  owner_pag[index_p].pid = -1;

  // liberar pages del proceso
  for (int  i = 0; i < capacity_pag; i++)
  {
    if(owner_pag[index_p].space[i].val != -1){
      free_page_frame(owner_pag[index_p].space[i].vpfn);
      free(owner_pag[index_p].space[i].free_list);
    }
  }
  free(owner_pag[index_p].space);  
}
