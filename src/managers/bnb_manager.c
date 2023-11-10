#include "bnb_manager.h"

#include "stdio.h"
#include "stdlib.h"

int bound ;

int process_base [20];
int process_sp [20];
int *process_heap_freelist [20]; // como es el size de esto

int memory_free_list [20];

int current_process_pid;

int active_process[20];



// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  
  bound = m_size()/20;
  
   for(int i = 0 ; i < 20; i++)
  {
   memory_free_list[i] = -1 ;
   active_process[i] = 0;
  }

  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.

int m_bnb_malloc(size_t size, ptr_t *out) {
  
  int c = size;

 for(int i = 0; i < bound/2; i++)
 {
   if(process_heap_freelist[current_process_pid][i] == 0)
   {
    c--;
   }
   else
   {
    c = size;
   }

   if (c == 0)
   {
   
    for(int j = i-size+1; j <= i; j++)
    {    
      process_heap_freelist[j] = 1;
    }
   
   out->addr = (process_base[current_process_pid]+(i-size+1));
   out->size = size;
    return 0;
   }
 }

 return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

//chequear si direccin dentro de bloque de programa

if(ptr.addr >= process_base[current_process_pid] && ptr.addr <= process_base[current_process_pid]+bound )
{

int ptrv_addr = ptr.addr - process_base[current_process_pid];

for(int i = 0; i < ptr.size; i++)
{
 
if(process_heap_freelist[current_process_pid][ptrv_addr+i]!=1)
{
  return 1;
}

}

for(int i = 0; i < ptr.size; i++)
{
  process_heap_freelist[current_process_pid][ptrv_addr+i] = 0;
}

return 0;
}
return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  
  if(process_sp[current_process_pid]-1 >= bound/2)
  {

  process_sp[current_process_pid]--;
  m_write(process_sp[current_process_pid], val);
  out = (process_sp[current_process_pid]);
  

  return 0;
  }
  else
  {
    return 1;
  }
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  
 if(process_sp[current_process_pid] < (current_process_pid+1)*bound)
 {
  out = m_read(process_sp[current_process_pid]);
  process_sp[current_process_pid]++;
  return 0;
 }

 return 1;

}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  
  if(addr >= process_base[current_process_pid] && addr <= process_base[current_process_pid]+bound )
  {
   out = m_read(addr);
   return 0;
  }  
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  if(addr >= process_base[current_process_pid] && addr <= process_base[current_process_pid] + bound )
  {
  
  int addr_v = addr - process_base[current_process_pid];
  
  if(process_heap_freelist[current_process_pid][addr_v] = 0)
  {
    process_heap_freelist[current_process_pid][addr_v] = 1;
    m_write(addr,val);
    return 0;
  }
    
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {



  FILE *a = fopen("angel.txt","+r");
  fprintf(a,"anfa");
  fclose(a);

 
if(active_process[process.pid] == 0)
{
  active_process[process.pid] = 1;

  for(int i = 0; i < 20; i++)
  {
    if(memory_free_list[i] == -1)
    {
      memory_free_list[i] = process.pid;
      break;
    }
  }

  process_base[process.pid] = process.pid*bound;

  process_sp[process.pid] = (process.pid+1)* bound;
  
  m_set_owner(process_base[process.pid],process_base[process.pid]+bound-1);

  process_heap_freelist[process.pid] = (int*) malloc (bound*(sizeof(int)));

  for(int i = 0 ; i < bound/2; i++)
  {
    process_heap_freelist[process.pid][i] = 0; 
  }
}

current_process_pid = process.pid;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  active_process[process.pid] = 0;

  m_unset_owner(process_base[process.pid],process_base[process.pid]+bound-1);

  for(int i = 0; i < 20; i++)
  {
    if(memory_free_list[i] == process.pid)
    {
      memory_free_list[i] = -1;
    }
  } 
    
}
