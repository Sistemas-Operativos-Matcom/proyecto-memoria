#include "bnb_manager.h"

#include "stdio.h"
#include "stdlib.h"

const size_t bound=30;

typedef struct DirectionSpace{

    ptr_t stack;
    byte *base;
    ptr_t end;
    int *freelist;
    process_t procces;

}space;

space InicialiceSpace(process_t process){

  
    
    space aux;
    aux.base= (byte*)malloc(sizeof(byte)*bound);
    aux.end.addr=(addr_t)&(aux.base[bound]);
    aux.stack.addr=aux.end.addr;
    aux.procces=process;
    aux.freelist= (int*)malloc(sizeof(int)*bound);
    
    return aux;
}

typedef struct SpaceList
{
    space* data;
    size_t size;
    size_t index;

}spaceList;

spaceList InicialiceList(){

   spaceList list;
   
   (list).data= (space*) malloc(sizeof(space)*10);
   (list).size=10;
   (list).index=0;

   return list;

}

void GrowList(spaceList *list){

    space* aux=(*list).data;
    space* var= (space*) malloc(sizeof(space)*(*list).size*2);
    for (size_t i = 0; i < (*list).size; i++)
    {
        var[i]=aux[i];
    }
    
    (*list).data=var;
    (*list).size=(*list).size+2;
    free(aux);

}

void ListAdd(spaceList *list,space value){
    if((*list).size==(*list).index){
        GrowList(list);
    }

    (*list).data[(*list).index]=value;
   
    (*list).index++;
    
}



void ListDel(spaceList *list,size_t index){

    space* aux=(*list).data;
    space* var= (space*) malloc((sizeof(space)*(*list).size)-1);

    for (size_t i = 0,j=0; i < (*list).size; i++,j++)
    {
        if(i!=index){
            var[j]=aux[i];
        }
        if(i==index){
            i++;
            var[j]=aux[i];
        }
    }
    (*list).data=var;
    (*list).index--;
    (*list).size--;
    

}

spaceList *spaces;
size_t indspace;

int FindFreeSpace(size_t size){


    for (size_t i = 0; i < bound; i++)
    {
        size_t count=0;
        for (size_t j = 0; i < size; j++)
        {
            if(spaces->data[indspace].freelist[i+j]==0) count++;
        }
        if(count==size) return i;
    }

    return -1;
    
}

int FindIndex(ptr_t ptr){

     for (size_t i = 0; i < bound; i++)
  {
    if((addr_t)&(spaces->data[indspace].base[i])==ptr.addr) return i;
  }

  return -1;
}

int FindAddress(addr_t addr){

      for (size_t i = 0; i < bound; i++)
  {
    if((addr_t)&(spaces->data[indspace].base[i])==addr) return i;
  }

  return -1;
}





// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {


  *spaces = InicialiceList();

}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {

  size_t index= FindFreeSpace(size);

  if(index==(size_t)-1) return 1;

  for (size_t i = 0; i < size; i++)
  {
    spaces->data[indspace].freelist[index+i]=1;
  }
  
  out->addr= (addr_t)&(spaces->data[indspace].base[index]) ;

  return 0;

}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {

  size_t index=FindIndex(ptr);

  if(index==(size_t)-1) return 1;

  spaces->data[indspace].freelist[index]=0;

  return 0;
  
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  
  size_t index= FindIndex(spaces->data[indspace].stack);

  if(spaces->data[indspace].freelist[index]==1) return 0;

  spaces->data[indspace].base[index]=val; 

  out->addr=(addr_t)&(spaces->data[indspace].base[index]);

  spaces->data[indspace].stack.addr--;
  
  return 1;

}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {

  if((spaces->data[indspace].stack.addr)==spaces->data[indspace].end.addr) return 1;

  size_t index= FindIndex(spaces->data[indspace].stack);

  *out= spaces->data[indspace].base[index+1];

  spaces->data[indspace].stack.addr++;

  return 0;
  
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {

  size_t index= FindAddress(addr);

  if(index==(size_t)-1) return 1;

  *out= spaces->data[indspace].base[index];

  return 0;
  
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {

  size_t index=FindAddress(addr);
  
  if(index==(size_t)-1) return 1;

  spaces->data[indspace].base[index]=val;

  return 0;

}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {

  for (size_t i = 0; i < spaces->size; i++)
  {
    if(spaces->data->procces.pid==process.pid) indspace=i;
  }

  ListAdd(spaces,InicialiceSpace(process));
  
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {

  for (size_t i = 0; i < spaces->size; i++)
  {
    if(spaces->data->procces.pid==process.pid) ListDel(spaces,i);
  }
  
 
}
