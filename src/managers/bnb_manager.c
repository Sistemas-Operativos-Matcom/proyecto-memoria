#include "bnb_manager.h"
#include <limits.h>
#include "stdio.h"

//Lista de procesos
static node_item_t* ListadeProcesos;
static int curr_pid;
static int curr_process_size;
static int bound;
//Free list para manejar el espacio libre de la memoria fisica
static node_t* m_free_list;





// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  //inicializa las variables y estructuras
  ListadeProcesos=NULL;
  m_free_list=NULL;
  insert_freelist(&m_free_list,0,m_size());
  curr_pid=-1;
  curr_process_size=-1;
  bound=700;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
  
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;

  while (temp != NULL){
    
    
    if(temp->item.process.pid == curr_pid){
      //elimina el espacio reservado de la free_list
      addr_t reserver_addr= less_freelist(&temp->item.free_list,size);
      
      if((int)reserver_addr==-1) {
        return 1;
      }

      out->addr=reserver_addr;
      
      return 0;
    }
    temp=temp->next;
  }
  
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr){
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;
  while (temp != NULL){
    if(temp->item.process.pid == curr_pid){
      
      if(ptr.addr <=temp->item.process.program->size -1) return 1;
      if(ptr.addr + ptr.size >= temp->item.stack_pointer ) return 1;
      //inserta en la free list el nuevo espacio libre
      insert_freelist(&temp->item.free_list,ptr.addr,ptr.size);
      return 0;
    }
    temp=temp->next;
  }
  return 1;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;
  while (temp != NULL){
    if(temp->item.process.pid == curr_pid){
      //realiza el push
      int validAgregation= push_freelist(&temp->item.free_list, &temp->item.stack_pointer);
      if(validAgregation==1) return 1;
     
      out->addr=temp->item.stack_pointer;
      //escribe en la memoria fisica el valor pusheado
      m_write(temp->item.base+temp->item.stack_pointer,val);
      
      return 0;
      
    }
    temp=temp->next;
  }
  
  return 1;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;
  while (temp != NULL){
    if(temp->item.process.pid == curr_pid){
      //realiza el pop
      return pop_freelist(&temp->item.free_list,&temp->item.stack_pointer,out,temp->item.base+temp->item.bound-1,temp->item.base);
    }
    temp=temp->next;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;
  while (temp != NULL){
    if(temp->item.process.pid == curr_pid){
      
      node_t* temp1 = temp->item.free_list;
      //comprueba si la direccion es valida
      while (temp1 != NULL){
        if(temp1->data.addr<=addr && addr < temp1->data.addr + temp1->data.size) return 1;
        temp1=temp1->next;
      }
      //lee el valor en la memoria fisica y modifica el valor al que apunta el puntero
      *out=m_read(addr+temp->item.base);
      return 0;
    }
    temp=temp->next;
  }

  return 1;

}




// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  //busca el proceso actual
  node_item_t* temp = ListadeProcesos;
  while (temp != NULL){
    if(temp->item.process.pid == curr_pid){
      //comprueba si la direccion es valida
      node_t* temp1 = temp->item.free_list;
      while (temp1 != NULL){

        if(temp1->data.addr<=addr && addr < temp1->data.addr + temp1->data.size) return 1;
        
        temp1=temp1->next;
      }

      //escribe el valor en la memoria fisica
      m_write(addr+temp->item.base,val);
      return 0;
    }
    temp=temp->next;
  }
  
  return 1;
}


// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  
        
  //busca el proceso actual
  int isNew=1;
  node_item_t* temp=ListadeProcesos;
  while(temp!=NULL){
    if(temp->item.process.pid == process.pid){
      isNew=0;
    }
    temp=temp->next;
  }
  curr_pid=process.pid;
  if(isNew == 0)return;
  
  //si el proceso es nuevo busca una direccion de memoria donde almacenarlo, inicializa el proceso y lo agrega a la lista de procesos
  addr_t base= less_freelist(&m_free_list,bound);

  node_t* process_free_list=(node_t*)malloc(sizeof(node_t));
  process_free_list->data.addr=process.program->size;
  process_free_list->data.size=bound-process.program->size;
  process_free_list->next=NULL;
  process_free_list->prev=NULL;

  item_t NewProcess= {base,bound,process,process_free_list,bound-1};
  push_item(&ListadeProcesos,NewProcess);
  m_set_owner(base, base+bound-1);
  
  return;
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {

  //busca el proceso actual
  node_item_t* temp=ListadeProcesos;
  
  while(temp!=NULL){
    if(temp->item.process.pid == process.pid){
      //elimina el proceso y libera su espacio
      curr_pid=-1;
      insert_freelist(&m_free_list,temp->item.base,temp->item.bound);
      m_unset_owner(temp->item.base,temp->item.base + temp->item.bound-1);
      delete_item(&ListadeProcesos,process.pid);
      
      return;
    }
    temp=temp->next;
  }
  return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void push_item(node_item_t** head, item_t item){

  node_item_t* new_node= (node_item_t*)malloc(sizeof(node_item_t));
  new_node->item= item;
  new_node->prev=NULL;
  if(*head==NULL){
    new_node->next=NULL;
    (*head)= new_node;
    return;
  }
  new_node->next=(*head);
  (*head)->prev=new_node;
  (*head)=new_node;


}


void delete_item(node_item_t** head, int pid){

  node_item_t* temp = *head;
  
  while (temp != NULL){
    if(temp->item.process.pid == pid){
      
      
      
      if(temp->next != NULL){
      (temp->next)->prev= temp->prev;
      }
      if(temp == *head){
      (*head)=temp->next;
      }
      free(temp);
      return;
    }
    temp=temp->next;
  }
}

void beggining_freelist(node_t** head, addr_t addr, int size) {

  free_space_t new_data={addr,size};
  node_t* new_node = (node_t*) malloc(sizeof(node_t));
  new_node->data = new_data;
  new_node->next = (*head);
  (*head)->prev = new_node;
  (*head) = new_node;
  
}

void insert_freelist(node_t** head, addr_t addr, int size){
  
  //si la free list esta vacia inserta el espacio
  if (*head ==NULL){
    node_t* new_node = (node_t*) malloc(sizeof(node_t));
    free_space_t new_data={addr,size};
    new_node->data = new_data;
    new_node->prev=NULL;
    new_node->next=NULL;
    
    (*head)= new_node;

    return;
  }
  // si la free list esta ocupado busca los nodos con los que podrian mezclarse los espacios libres

  node_t* temp = *head;
  node_t* predecesor = NULL;
  while(temp!= NULL ){
    if(temp->data.addr <= addr ){
      predecesor = temp;
      
    }
    temp= temp->next;
  }
  if(predecesor==NULL){
    beggining_freelist(head,addr,size);
    return;
  }

  //si se pueden mezclar los espacios libres se hace
  if(predecesor->data.addr + predecesor->data.size == addr){
    predecesor->data.size = predecesor->data.size + size;
    if(predecesor->next !=NULL && predecesor->data.addr + predecesor->data.size == (predecesor->next)->data.addr){
      predecesor->data.size = predecesor->data.size+ (predecesor->next)->data.size;
      if((predecesor->next)->next != NULL){
        predecesor->next = (predecesor->next)->next;
        free((predecesor->next)->prev);
        (predecesor->next)->prev = predecesor;
      }
      else{
        free(predecesor->next);
        predecesor->next=NULL;
      }
      
    }
    return;
  }

  node_t* new_node = (node_t*) malloc(sizeof(node_t));
  free_space_t new_data = {addr,size};
  new_node->data = new_data;
  new_node->prev = predecesor;
  new_node->next = predecesor->next;
  predecesor->next = new_node;
  if(new_node->next != NULL){
    (new_node->next)->prev= new_node;
  }
  if(new_node->next !=NULL && new_node->data.addr + new_node->data.size == (new_node->next)->data.addr){
    new_node->data.size = new_node->data.size+ (new_node->next)->data.size;
    if((new_node->next)->next != NULL){
        new_node->next = (new_node->next)->next;
        free((new_node->next)->prev);
        (new_node->next)->prev = new_node;
      }
      else{
        free(new_node->next);
        new_node->next=NULL;
      }
  }
  return;

}

addr_t less_freelist(node_t** head, int size){
  node_t* temp = *head;
  int min= INT_MAX;
  node_t* space= NULL;
  addr_t resp;

  
  while (temp != NULL) {
    
    if(temp->data.size - size >= 0 && temp->data.size - size < min){
      
      min= temp->data.size - size;
      space=temp;
    }
    temp=temp->next;
  }
  //si se puede reservar ese espacio en la free list lo hace
  if(space!=NULL){

    if(min!=0){
      
      space->data.addr= space->data.addr +size;
      space->data.size= space->data.size -size;
      
      return space->data.addr-size;
    }
    
    if(space->prev != NULL){
      (space->prev)->next= space->next;
    }
    if(space->next != NULL){
      (space->next)->prev= space->prev;
    }

    if(space == *head){
      (*head)=space->next;
    }
    resp= space->data.addr-size;
    
    free(space);
    return resp;
    
  }
  
  return -1;
  
}

int push_freelist(node_t** head,addr_t* stack_pointer){
  node_t* temp = *head;
  if(temp==NULL) return 1;
  while(temp->next!= NULL ){
    temp= temp->next;
  }
  if(temp->data.addr + temp->data.size-1 != *stack_pointer) return 1;
  temp->data.size-=1;
  *stack_pointer-=1;
  if(temp->data.size==0){
    (temp->prev)->next=NULL;
    free(temp);
  }
  return 0;
}

int pop_freelist(node_t** head,addr_t* stack_pointer, byte* out, int process_end,addr_t base){
  if((int)*stack_pointer==process_end) return 1;
  *out=m_read(*stack_pointer + base);
  insert_freelist(head,*stack_pointer,1);
  *stack_pointer=*stack_pointer+1;
  return 0;
}



