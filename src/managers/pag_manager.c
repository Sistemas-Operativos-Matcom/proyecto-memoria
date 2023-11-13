#include "pag_manager.h"

#include "stdio.h"
#include "structs.h"

int proc;
Page_t *Pages;
Proc_t *Procs;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  Pages = (Page_t *) malloc(m_size()/32*sizeof(Page_t));
  Procs = (Proc_t *) malloc(1000*sizeof(Proc_t));

  proc = -1;

  int base = 0;

  for (size_t i = 0; i < m_size()/32; i++)
  {
    Pages[i].inUse = 0;
    Pages[i].base = base;
    base += 32;
    for (size_t j = 0; j < 32; j++)
    {
      Pages[i].bytes[j] = 0;
    }
  }

  for (size_t i = 0; i < 1000; i++)
  {
    Procs[i].pid = -1;
    Procs[i].lpcode = -1;
    Procs[i].lpstack = -1;
    Procs[i].lpheap = -1;
    Procs[i].code = (Page_t *)malloc(100 * sizeof(Page_t ));
    Procs[i].stack = (Page_t *)malloc(100 * sizeof(Page_t ));
    Procs[i].heap = (Page_t *)malloc(100 * sizeof(Page_t ));
  }
  
  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  if (size > 32 || Procs[proc].lpheap == -1)
  // si el heap esta vacio o el espacio a reservar es mayor que el tamaño de una pagina
  {
    // cantidad de paginas vacias consecutivas
    int len = 0;
    // indice del inicio de las paginas vacias
    int count = 0;
    for (size_t i = 0; i < m_size()/32; i++)
    {
      if (!Pages[i].inUse)
      {
        len ++;
        count = i;
      }
      else 
      {
        len = 0;
        count = 0;
      }
      if (len*32 >= size)
      {
        count = count - len +1;
        break;
      }
    }
    if (len*32 < size)
    {
      return 1;
    }
    for (size_t i = count; i < count + len; i++)
    // asigna las paginas al proceso
    {
      Procs[proc].lpheap++;
      Procs[proc].heap[Procs[proc].lpheap] = Pages[i];
      Pages[i].inUse = 1;
      m_set_owner(Pages[i].base, Pages[i].base + 31);
      for (size_t j = 0; j < 32; j++)
      {
        Pages[i].bytes[j] = 1;
      }
      
    }
    Procs[proc].lpheap++;
    Procs[proc].heap[Procs[proc].lpheap] = Pages[count + len +1];
    m_set_owner(Pages[count + len +1].base, Pages[count + len +1].base + 31);
    Pages[count + len +1].inUse = 1;
    for (size_t j = 0; j < size%32; j++)
    {
      Pages[count + len +1].bytes[j] = 1;
    }

    out->addr = Pages[count].base;
    out->size = size;
    return 0;
  }

  // si size es menor que el tamaño de la pagina busca entre las paginas ya usadas alguna que tenga 
  // espacio disponible

  int len; 
  int count;
  for (size_t i = 0; i < Procs[proc].lpheap +1; i++)
  {
    for (size_t j = 0; j < 32; j++)
    {
      if(Procs[proc].heap[i].bytes[j] == 0)
      {
        len ++;
        count = i;
      }
      else{
        len = 0;
        count = 0;
      }
      if (len == size)
      {
        count = count - len +1;
        break;
      }
    }
    if (len != size)
    {
      continue;
    }
    for (size_t j = count; j < count + len; j++)
    {
      Procs[proc].heap[i].bytes[j] = 1;
    }
    out->addr = Procs[proc].heap[i].base + count;
    out->size = size;
    return 0;
  }

  // si ninguna de las paginas del heap tienen espacio suficiente se usa una nueva pagina

  for (size_t i = 0; i < m_size()/32; i++)
  {
    if(!Pages[i].inUse)
    {
      Pages[i].inUse = 1;
      m_set_owner(Pages[i].base,Pages[i].base+31);
      Procs[proc].lpheap++;
      Procs[proc].heap[Procs[proc].lpheap] = Pages[i];
      for (size_t j = 0; j < size; j++)
      {
        Procs[proc].heap[i].bytes[j] = 1;
      }
      out->addr = Pages[i].base;
      out->size = size;
      return 0;
    }
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  if (ptr.size <= 32)
  {
    for (size_t i = 0; i < ptr.size; i++)
    {
      Pages[ptr.addr/32].bytes[ptr.addr%32 + i] = 0;
    }
    
    return 0;
  }
  int size = ptr.size;
  for (size_t i = 0; i < size/32; i++)
  {
    for (size_t j = 0; j < 32; j++)
    {
      Pages[ptr.addr/32 + i].bytes[j] = 0;
    }
    
    size -= 32;
  }
  if (size > 0)
  {
    for (size_t i = 0; i < size; i++)
    {
      Pages[ptr.addr/32 + ptr.size/32].bytes[i] = 0;
    }
    
  }
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if (Procs[proc].lpstack == -1)
  // si no hay ninguna pagina de stack
  {
    for (size_t i = 0; i < m_size()/32; i++)
    {
      if (!Pages[i].inUse)
      {
        Pages[i].inUse = 1;
        m_set_owner(Pages[i].base, Pages[i].base + 31);
        Procs[proc].lpstack++;
        Procs[proc].stack[Procs[proc].lpstack] = Pages[i];
        break;
      }
    }
  }
  for (size_t i = 0; i < 32; i++)
  // busca el 1er 0 en la ultima pagina de stack
  {
    if (Procs[proc].stack[Procs[proc].lpstack].bytes[i] == 0)
    {
      m_write(Procs[proc].stack[Procs[proc].lpstack].base + i,val);
      Procs[proc].stack[Procs[proc].lpstack].bytes[i] = 1;
      out->addr = Procs[proc].stack[Procs[proc].lpstack].base + i;
      out->size = 1;
      return 0;
    }
  }
  for (size_t i = 0; i < m_size()/32; i++)
  // si la ultima pagina de stack esta llena se añade una nueva
  {
    if (!Pages[i].inUse)
    {
      Pages[i].inUse = 1;
      m_set_owner(Pages[i].base, Pages[i].base + 31);
      Procs[proc].lpstack++;
      Procs[proc].stack[Procs[proc].lpstack] = Pages[i];
      Procs[proc].stack[Procs[proc].lpstack].bytes[0] = 1;
      m_write(Procs[proc].stack[Procs[proc].lpstack].base,val);
      out->addr = Procs[proc].stack[Procs[proc].lpstack].base;
      out->size = 1;
      return 0;
    }
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  // busca el ultimo 1 de la ultima pagina de stack
  for (size_t i = 31; i >= 0; i--)
  {
    if (Procs[proc].stack[Procs[proc].lpstack].bytes[i])
    {
      *out = m_read(Procs[proc].stack[Procs[proc].lpstack].base + i);
      Procs[proc].stack[Procs[proc].lpstack].bytes[i] = 0;
      if (i == 0)
      {
        // si la pagina se queda vacia pasa a estar disponible
        Procs[proc].stack[Procs[proc].lpstack].inUse = 0;
        m_unset_owner(Procs[proc].stack[Procs[proc].lpstack].base, Procs[proc].stack[Procs[proc].lpstack].base + 31);
        Procs[proc].lpstack --;
      }
      return 0;
    }
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  *out = m_read(addr); 
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  m_write(addr,val); 
  return 0;
}

// Notifica un cambio de contexto al proc 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  int is = 0;
  for (size_t i = 0; i < 1000; i++)
  {
    if (process.pid == Procs[i].pid)
    {
      proc = i;
      is = 1;
      break;
    }
  }
  if (!is)
  // si el proceso no habia sido ejecutado anteriormente guarda espacio para su codigo
  {
    for (size_t i = 0; i < 1000; i++)
    {
      if(Procs[i].pid == -1)
      {
        proc = i;
        Procs[i].pid = process.pid;
        break;
      }
    }
    int code = process.program->size;
    int len = 0;
    int count = 0;
    for (size_t i = 0; i < m_size()/32; i++)
    {
      if (!Pages[i].inUse)
      {
        len ++;
        count = i;
      }
      else 
      {
        len = 0;
        count = 0;
      }
      if (len*32 >= code)
      {
        count = count - len +1;
        break;
      }
    }
    if (len*32 < code)
    {
      return 1;
    }
    for (size_t i = count; i < count + len; i++)
    {
      Procs[proc].lpcode++;
      Procs[proc].code[Procs[proc].lpcode] = Pages[i];
      Pages[i].inUse = 1;
      for (size_t j = 0; j < 32; j++)
      {
        Pages[i].bytes[j] = 1;
      }
      
    }
    Procs[proc].lpcode++;
    Procs[proc].code[Procs[proc].lpcode] = Pages[count + len +1];
    Pages[count + len +1].inUse = 1;
    for (size_t j = 0; j < code%32; j++)
    {
      Pages[count + len +1].bytes[j] = 1;
    }

    m_set_owner(Pages[count].base, Pages[count].base+code);
    return 0;
    
    
  }
}

// Notifica que un proc ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  //limpiar las estructuras para que puedan ser utilizadas por otros procesos
  for (size_t i = 0; i < 1000; i++)
  {
    if (process.pid == Procs[i].pid)
    {
      Procs[i].pid = -1;
      for (size_t j = 0; j < Procs[j].lpcode +1; j++)
      {
        Procs[i].code[j].inUse = 0;
        m_unset_owner(Procs[i].code[j].base, Procs[i].code[j].base + 1);
        for (size_t k = 0; k < 32; k++)
        {
          Procs[i].code[j].bytes[k] = 0;
        }
      }
      for (size_t j = 0; j < Procs[j].lpstack +1; j++)
      {
        Procs[i].stack[j].inUse = 0;
        m_unset_owner(Procs[i].stack[j].base, Procs[i].stack[j].base + 1);
        for (size_t k = 0; k < 32; k++)
        {
          Procs[i].stack[j].bytes[k] = 0;
        }
      }
      for (size_t j = 0; j < Procs[j].lpheap +1; j++)
      {
        Procs[i].heap[j].inUse = 0;
        m_unset_owner(Procs[i].heap[j].base, Procs[i].heap[j].base + 1);
        for (size_t k = 0; k < 32; k++)
        {
          Procs[i].heap[j].bytes[k] = 0;
        }
      }

      Procs[i].lpcode = -1;
      Procs[i].lpstack = -1;
      Procs[i].lpheap = -1;

      return 0;
    }
  }
  return 1;
}
