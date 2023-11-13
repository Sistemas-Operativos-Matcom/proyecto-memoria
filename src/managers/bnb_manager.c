#include "bnb_manager.h"

#include "stdio.h"
#define OUT_OF_LIMITS 20000ul
#define NO_FREE_MEMORY OUT_OF_LIMITS + 1ul
#define UL unsigned long


addr_t vmem_size = 256ul;
process_t curr_process;
unsigned long sections_amount;
int sections[2000];
UL curr_section;
UL stack_pointer[2000];
int free_mem[2000][256];

addr_t address_translator(addr_t v_address){
  if(v_address < vmem_size){
   return (curr_section * vmem_size) + v_address;

  }else{
    return OUT_OF_LIMITS;
  }
 

}

addr_t search_free(size_t size){

  unsigned int counter = 0u;

  for(addr_t c = 0ul ; c < stack_pointer[curr_section];c++){

    if(free_mem[curr_section][c] == FREE){

      counter++;

    }else{

      counter = 0u;
    }

    if(counter == size){

      for(unsigned long d = c - size;d < c;d++){

        free_mem[curr_section][d] = USED;

      }

      return c - size + 1ul;

    }

  }

  return NO_FREE_MEMORY;

}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  sections_amount = m_size() / vmem_size;
  
  for(UL c = 0ul;c < sections_amount;c++){
    
    sections[c] = -1;

  }
  for(UL c = 0ul;c < sections_amount;c++){
    
    stack_pointer[c] = vmem_size;

  }
  for(UL sec = 0ul;sec < sections_amount;sec++){

    for(int pos = 0 ;pos < vmem_size;pos++){
    
    free_mem[sec][pos] = FREE;

    }

  }
  

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  addr_t v_address = search_free(size);
  if(v_address != NO_FREE_MEMORY){
    
   (*out).addr = v_address;
   (*out).size = size;
   
   return 0;

  }

 return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {//implementar

  if((ptr.addr < curr_section * vmem_size) ||( ptr.addr >= (curr_section + 1u) * vmem_size ))return 1;

  for(addr_t c = 0u;c < ptr.size;c++){
   free_mem[curr_section][ptr.addr % vmem_size + c] = FREE;
  }
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {

  if(free_mem[curr_section][stack_pointer[curr_section] - 1ul] == USED){
    return 1;
  }

  stack_pointer[curr_section]--;
  free_mem[curr_section][stack_pointer[curr_section]] = USED;
  addr_t real_address = address_translator(stack_pointer[curr_section]);
  m_write(real_address,val);
  (*out).size = 1ul;
  (*out).addr = stack_pointer[curr_section];

  return 0;
}//fin de push

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  if(stack_pointer[curr_section] >= vmem_size){

    return 1;

  }else{
    
    *out = m_read( address_translator( stack_pointer[curr_section] ) );

    stack_pointer[curr_section]++;

    return 0;

  }

}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  int return_value = m_read(address_translator(addr));

  if(return_value == OUT_OF_LIMITS){

    return 1;

  }else{

    *out = return_value;

    return 0;

  }

 
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  
   int address = address_translator(addr);
  
  if(address == OUT_OF_LIMITS){
    return 1;

  }else{

    m_write(address,val);

    return 0;

  }

}

unsigned long search_free_section(int pid){
  UL result = 0;
  int found_free = 0;
  for(int c = 1 ;c < sections_amount;c++){
   
   if(sections[c] == -1 && found_free == 0){
      result = c;
      found_free = 1;
     }

   if(sections[c] == pid){

      return c;

     }

  }
  sections[result] = pid;
  
  return result;

}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  
  UL section = search_free_section(process.pid);

    curr_section = section;

    m_set_owner(curr_section * vmem_size ,( curr_section + 1ul ) * vmem_size - 1ul);

       

}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {

  sections[curr_section] = -1;

  m_unset_owner(curr_section * vmem_size,( curr_section + 1ul )* vmem_size - 1ul);


}
