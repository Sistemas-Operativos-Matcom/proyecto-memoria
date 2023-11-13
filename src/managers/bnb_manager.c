#include "bnb_manager.h"

#include "stdio.h"

typedef struct owner{
  int pid;
  int base;
  int bounds;
} owner_t;

typedef struct space{
  int stack_pointer;
  int heap_base;
  int heap_bounds;
  int *virtual_mem;
} space_t;

owner_t *owners;
space_t *spaces;
int curr_pid;
int capacity;

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  curr_pid = -1;
  // Inicializar owners
  capacity = m_size()/1024;
  owners = (owner_t *)malloc(capacity * sizeof(owner_t));
  spaces = (space_t *)malloc(capacity * sizeof(space_t));

  for(int i = 0; i < capacity; i ++){
    owners[i].pid = -1;
  } 
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }

  // buscar size espacios libres en heap
  int count = 0;
  int lenght = spaces[index_p].heap_base + spaces[index_p].heap_bounds;
  for(int i = spaces[index_p].heap_base; i < lenght; i++){
    
    if(spaces[index_p].virtual_mem[i] == -1){
      count ++;
      int val = 1;
      while(i+val < lenght && spaces[index_p].virtual_mem[i + val] == -1){
        count ++;
        val ++;
      }
      if(count >= (int)size){
        // va
        out->addr = (size_t)i;

        // rellenar 1
        for(int r = i; r < count; r ++){
          spaces[index_p].virtual_mem[r] = 1;
        }

        return 0;
      }else{
        i = i + count -1;
        count = 0;
      }
    }
  }
  // no espacio libre
  if(spaces[index_p].heap_base +spaces[index_p].heap_bounds + (int)size >= spaces[index_p].stack_pointer){
    return 1;
  }else {
    out->addr = spaces[index_p].heap_base + spaces[index_p].heap_bounds;

    int pos = spaces[index_p].heap_base + spaces[index_p].heap_bounds;
    for(int t = pos; t < pos + (int)size; t ++){
      spaces[index_p].virtual_mem[t] = 1;
    }
    return 0;
  }

  return 1; 
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }
  int length = (int)ptr.addr + (int)ptr.size;
  for (int i = (int)ptr.addr; i < length; i++)
  {
    if (spaces[index_p].virtual_mem[i] == -1)
    {
      return 1;
    }
    
    spaces[index_p].virtual_mem[i] = -1;
  }
  
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  
  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }
  
  
  if(spaces[index_p].virtual_mem[spaces[index_p].stack_pointer - 1] != -1)
  {
    
    return 1;
  }else {
    spaces[index_p].stack_pointer --;
    spaces[index_p].virtual_mem[spaces[index_p].stack_pointer] = 2;
    int new_addr = owners[index_p].base + spaces[index_p].stack_pointer;
    m_write(new_addr, val);
    out->addr =  spaces[index_p].stack_pointer;

    return 0;    
  }
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
   int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }

  if(spaces[index_p].stack_pointer == 1024){
    return 1;
  }
  int new_addr = owners[index_p].base + spaces[index_p].stack_pointer;
  *out = m_read(new_addr);
  spaces[index_p].stack_pointer ++;

  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }

  if(spaces[index_p].virtual_mem[addr] != -1){
    int new_addr = owners[index_p].base + addr;
    *out = m_read(new_addr);
    return 0;
  }else{
    return 1;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == curr_pid){
      index_p = p;
      break;
    }
  }

  if(spaces[index_p].virtual_mem[addr] == 1)
  {
    // convertir a pa
    m_write(owners[index_p].base + addr, val);
    return 0;
  }
  else{
    return 1;
  }  
  return 1;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  curr_pid = process.pid;
  
  
  // buscar el pid en owners
  for(int i = 0; i < capacity; i ++){
    if(curr_pid == owners[i].pid){
      return;
    }    
  }
  // insertar el pid en owners
  for(int i = 0; i < capacity; i ++){
    if(owners[i].pid == -1){
      owners[i].pid = curr_pid;
      owners[i].base = i*1024;
      owners[i].bounds = 1023;

      // actualizar el espacio de memoria virtual para el nuevo proceso
      spaces[i].heap_base = (int)process.program->size;
      spaces[i].heap_bounds = 0;
      spaces[i].stack_pointer = owners[i].bounds;


      spaces[i].virtual_mem = (int *)malloc(1024 * sizeof(int));
      
      for(int j = 0; j < 1024; j ++){
        if(j < spaces[i].heap_base){
          // espacio ocupado por codigo
          spaces[i].virtual_mem[j] = 0;
        }else if(j >= spaces[i].heap_base && j <= spaces[i].heap_base + spaces[i].heap_bounds){
          // espacio ocupado por el heap
          spaces[i].virtual_mem[j] = 1;
        }else {
          // espacio libre
          spaces[i].virtual_mem[j] = -1;
        }
       }

       // set new owner
       m_set_owner(i*1024,i*1024 + 1023);

      return;
    }    
  }

   
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
  int index_p = -1;
  for(int p = 0; p< capacity; p++){
    if(owners[p].pid == process.pid){
      index_p = p;
      break;
    }
  }
  owners[index_p].pid = -1;
  free(spaces[index_p].virtual_mem);
  // ownership
  m_unset_owner(owners[index_p].base, owners[index_p].base + owners[index_p].bounds);
  
}
