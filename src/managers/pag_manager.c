#include "pag_manager.h"

#include "stdio.h"

int page_size = 32 ;
int pages_qtt;
int pag_bound;
int *process_page_table[20];
int pag_proc_sp[20];
int *proc_heap_fl[20]; // como es el size de esto

int *pag_mem_fl;

int pag_current_proc_pid;

int pag_active_procs[20];

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {

  pages_qtt = m_size()/page_size;
  pag_bound = m_size()/20;
  
  pag_mem_fl = (int *)malloc(pages_qtt * (sizeof(int)));
  
  //fprintf(stderr,"pagesqtt: %d, pag_bound: %d\n", pages_qtt,pag_bound);

  for (int i = 0; i < pages_qtt; i++)
  {
    pag_mem_fl[i] = -1;  
  }
  
  for (int i = 0; i < 20; i++)
  {
    pag_active_procs[i] = 0;
  }
} 

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  
  int c = size;
  
 


  //fprintf(stderr,"malloc");

  for (int i = 0; i < pag_bound / 2; i++)
  {
    if (proc_heap_fl[pag_current_proc_pid][i] == 0)
    {
      c--;
    }
    else
    {
      c = size;
    }

    if (c == 0)
    {
      int addr_actual = (i - size + 1);

      while(addr_actual < i+1)
      {
        if( process_page_table[pag_current_proc_pid][addr_page(addr_actual)] != -1 )
        {
          addr_actual += 32 - addr_line(addr_actual);
        } 
        else
        { 
          int f = -1;
          for(int j = 0; j < pages_qtt; j++)
          {
            if(pag_mem_fl[j] == -1)
            {
              f=j;
              process_page_table[pag_current_proc_pid][addr_page(addr_actual)] = j;
              pag_mem_fl[j] = pag_current_proc_pid;
              //fprintf(stderr,"malloc1: %d, %d, actual_addr: %d \n", j*page_size, (j+1)*page_size - 1, addr_actual);
              m_set_owner(j*page_size, (j+1)*page_size - 1);
              break;
            }
          }
          if(f == -1)
          {
            return 1;
          }
          //fprintf(stderr,"%d", addr_actual);
        }
          addr_actual += page_size;
      }

      addr_actual = i;
 
     

      if( process_page_table[pag_current_proc_pid][addr_page(addr_actual)] != -1 )
        {
          addr_actual += 32 - addr_line(addr_actual);
           
        } 
        else
        { 
          int f = -1;
          for(int j = 0; j < pages_qtt; j++)
          {
            if(pag_mem_fl[j] == -1)
            {
              f=j;
              process_page_table[pag_current_proc_pid][addr_page(addr_actual)] = j;
              pag_mem_fl[j] = pag_current_proc_pid;
              //fprintf(stderr,"malloc2: %d, %d, actual_addr: %d \n", j*page_size, (j+1)*page_size-1, addr_actual);
              m_set_owner(j*page_size, (j+1)*page_size - 1);
              break;
            }
          }
          if(f == -1)
          {
            return 1;
          }
        }

      for (int j = i - size + 1; j <= i; j++)
      {
        proc_heap_fl[pag_current_proc_pid][j] = 1;
      }

      out-> addr = (i - size + 1);
      out-> size = size;

     
      return 0;
    }
  }

  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {

  if (ptr.addr >= 0 && ptr.addr < pag_bound)
  {
    for (int i = 0; i < ptr.size; i++)
    {
      if (proc_heap_fl[pag_current_proc_pid][ptr.addr + i] != 1 );
      { 
        
        return 1;
      }
    }
    
    for (int i = 0; i < ptr.size; i++)      //es linea por linea??
    {
      if(process_page_table[pag_current_proc_pid][addr_page(ptr.addr+i)] == -1 )
      {
        return  1;
      }
    }
    
    int first_page = 0; // se puede liberar la primera pag
    
    int c = addr_line(ptr.addr);
     
    for(int i = 0 ; i < c; i++)
    {
      if(proc_heap_fl[pag_current_proc_pid][addr_page(ptr.addr)*page_size + i] == 1)
      {
        first_page = 0; 
        break;
      }
      else
      {
        first_page = 1;
      }
    }

    int last_page = 0;

    c = addr_line(ptr.addr + ptr.size);

    for(int i = c ; i < page_size; i++)
    {
      if(proc_heap_fl[pag_current_proc_pid][addr_page(ptr.addr + ptr.size) * page_size + i] == 1 )
      {
        last_page = 0;
        break;
      }
      else
      {
        last_page = 1;
      }
    }
 
    if(first_page == 1)
    {
      pag_mem_fl[process_page_table[pag_current_proc_pid][addr_page(ptr.addr)]] = -1;
      process_page_table[pag_current_proc_pid][addr_page(ptr.addr)] = -1;
    }
    if(last_page == 1)
    {
      pag_mem_fl[process_page_table[pag_current_proc_pid][addr_page(ptr.addr + ptr.size)]] = -1;
      process_page_table[pag_current_proc_pid][addr_page(ptr.addr + ptr.size)] = -1;
    }
    
    c = addr_page(ptr.addr)+1;

    for(int i = c; i < addr_page(ptr.addr + ptr.size); i++)
    {
      pag_mem_fl[process_page_table[pag_current_proc_pid][i]]=-1;
      process_page_table[pag_current_proc_pid][i] = -1;
    }

    for (int i = 0; i < ptr.size; i++)
    {
      proc_heap_fl[pag_current_proc_pid][ptr.addr + i] = 0;
    }
    
    
    return 0;
  }

  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {

 

  if (pag_proc_sp[pag_current_proc_pid] - 1 >= pag_bound / 2)
  {
  
    pag_proc_sp[pag_current_proc_pid]--;

       
  
    if(process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])] == -1)
    {
     //fprintf(stderr,"push: %d\n", addr_page(pag_proc_sp[pag_current_proc_pid]));
      int f = -1;
      for(int i = 0 ;i < pages_qtt; i++)
      {
       if(pag_mem_fl[i] == -1)
       {
        f=i;
        pag_mem_fl[i] = pag_current_proc_pid;
        process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])] = i;
        m_set_owner(i*page_size, (i+1)*page_size-1);
        break; 
       }
      }

      if(f == -1) return 1;
    }

  
    //printf("dir: %d, val: %d",process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])]*page_size + addr_line(pag_proc_sp[pag_current_proc_pid]), val);
      
    m_write(process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])]*page_size + addr_line(pag_proc_sp[pag_current_proc_pid]), val);
    out->addr = process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])]*page_size + addr_line(pag_proc_sp[pag_current_proc_pid]);
    out->size = 1;

   



    return 0;
  
  }
  else
  {
    return 1;
  }
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  
  if (pag_proc_sp[pag_current_proc_pid] < pag_bound)
  { 
    int prev_page = addr_page(pag_proc_sp[pag_current_proc_pid]);
    
    //printf("SP: %d\n", pag_proc_sp[pag_current_proc_pid]);
    //printf("mread: %d\n", m_read(process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])]*page_size + addr_line(pag_proc_sp[pag_current_proc_pid])));
    *out = m_read(process_page_table[pag_current_proc_pid][addr_page(pag_proc_sp[pag_current_proc_pid])]*page_size + addr_line(pag_proc_sp[pag_current_proc_pid]));
    pag_proc_sp[pag_current_proc_pid]++;
    
    if(prev_page != addr_page(pag_proc_sp[pag_current_proc_pid]) )
    {
      
     for(int i = (pag_bound/2-1); i >= 0; i--)
     {
      if(proc_heap_fl[i] != -1)
      {
       if(addr_page(i) == prev_page)
       {
        return 0;
       }
      }
     }
    
     m_unset_owner(process_page_table[pag_current_proc_pid][prev_page]*page_size, (process_page_table[pag_current_proc_pid][prev_page]+1)*page_size-1);
     pag_mem_fl[process_page_table[pag_current_proc_pid][prev_page]] = -1;
     process_page_table[pag_current_proc_pid][prev_page] = -1;
       
    }

    return 0;
  }

  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
   if (addr >= 0 && addr < pag_bound)
  {
    if((proc_heap_fl[pag_current_proc_pid][addr] == 1 || addr >= pag_proc_sp[pag_current_proc_pid]) )
    {
      if(process_page_table[pag_current_proc_pid][addr_page(addr)] != -1 && pag_mem_fl[process_page_table[pag_current_proc_pid][addr_page(addr)]] != -1)
      {
        //fprintf(stderr, "%d /", m_read(process_page_table[pag_current_proc_pid][addr_page(addr)]*32 + addr_line(addr)));

        *out = m_read(process_page_table[pag_current_proc_pid][addr_page(addr)]*32 + addr_line(addr));
         return 0;
      }
    }
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  
   /* if(pag_current_proc_pid==1)
      {
      for(int i = 0; i < 12; i++ )
      {
       fprintf(stderr,"%d /", process_page_table[1][i]);
      }    
      } */

    //fprintf(stderr,"%d /",addr);

  if (addr >= 0 && addr < pag_bound)
  {    
     

    if (proc_heap_fl[pag_current_proc_pid][addr] == 1 && process_page_table[pag_current_proc_pid][addr_page(addr)] != -1)
    {
      
      //fprintf(stderr,"page table: %d",pag_mem_fl[process_page_table[pag_current_proc_pid][addr_page(addr)]]*32 + addr_line(addr));
      
      /*if(pag_current_proc_pid==1)
      {
      for(int i = 0; i < 12; i++ )
      {
       fprintf(stderr,"%d /", process_page_table[1][i]);
      }    
      }*/

      //fprintf(stderr,"%d" ,pag_mem_fl[process_page_table[pag_current_proc_pid][addr_page(addr)]]*32);

      m_write(process_page_table[pag_current_proc_pid][addr_page(addr)]*32 + addr_line(addr), val);

//     
      return 0;
    }
  }
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {

   

  if (pag_active_procs[process.pid] == 0)
  {
      
    pag_active_procs[process.pid] = 1;

    pag_proc_sp[process.pid] = pag_bound;
     
    process_page_table[process.pid] = (int *)malloc( (pages_qtt/20) * (sizeof(int)));
     
    proc_heap_fl[process.pid] = (int *)malloc(pag_bound * (sizeof(int)));
   
     
    for (int i = 0; i < pag_bound / 2; i++)
    {
      proc_heap_fl[process.pid][i] = 0;
    }

     for(int j = 0; j < pag_bound/20; j++)
     {
       process_page_table[process.pid][j] = -1;  
     } 

    
  }

  pag_current_proc_pid = process.pid;

  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  pag_active_procs[process.pid] = 0;

  fprintf(stderr, "end ");
 for (int i = 0; i < pages_qtt; i++)
  {
    if (process_page_table[process.pid][i] != -1)
    {
      pag_mem_fl[process_page_table[process.pid][i]] = -1;
    }
  }

 
}

int addr_page(int addr) {

return (addr / page_size);
 
}


int addr_line(int addr) {

return addr % page_size;
 
}