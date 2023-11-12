#include "pag_manager.h"
#include <limits.h>
#include "stdio.h"

//Lista de procesos
static node_page_item_t* ListadeProcesos;
static int curr_pid;
//Free list para manejar el espacio libre de la memoria fisica
static node_t* m_free_list;
//tamano de pagina
static int page_size;
//cantidad maxima de paginas para un proceso
static int max_pages;


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  //inicializa las variables y estructuras
  ListadeProcesos=NULL;
  m_free_list=NULL;
  insert_freelist_pag(&m_free_list,0,m_size());
  curr_pid=-1;
  page_size=50;
  max_pages=14;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){

    if(temp->item.process.pid==curr_pid){
      int cant_pages=  size / page_size;
      int ult_page=size % page_size;
      if(ult_page !=0) cant_pages+=1;
      
      //si puede reservar ese espacio en alguna pagina lo hace
      page_list_t* search=temp->item.page_list_heap;
      while (search!=NULL)
      {
        addr_t valid=less_freelist_pag(&search->page.free_list,size);
        if((int)valid != -1){

          out->addr=valid+search->page.number*page_size;
          
          return 0;
        }
        search=search->next;
      }
      

      //si no cupo en alguna pagina lo inserta en nuevas paginas
      if(temp->item.page_count + cant_pages <= max_pages){
        int n=0;
        addr_t first_addr=-1;
        while (n<cant_pages)
        {
          addr_t m_addr= less_freelist_pag(&m_free_list,page_size);
          
          m_set_owner(m_addr,m_addr+page_size-1);
          
          node_t* free_list=NULL;
          if(n==cant_pages-1 && ult_page!=0){
            free_space_t new_data={ult_page ,page_size-ult_page};
            free_list = (node_t*) malloc(sizeof(node_t));
            free_list->data=new_data;
            
          }
          int number=temp->item.unpages->data;
          if(n==0) first_addr=(addr_t)number;
          delete_number(&temp->item.unpages,number);
          page_t page={number,m_addr,page_size,free_list};
          push_page(&temp->item.page_list_heap,page);
          temp->item.page_count+=1;
          n++;
        }
        out->addr= first_addr*page_size;
        
        return 0;
      }
      
      
    }
    temp=temp->next;
  }
  return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){

    if(temp->item.process.pid==curr_pid){

      //busca la pagina donde comienza el espacio a liberar y elimina las paginas correspondientes en dependencia del espacio a liberar
      int page=ptr.addr/page_size;
      int location= ptr.addr%page_size;
      int cant_pages=ptr.size/page_size;
      int ult_page=ptr.size%page_size;
      while (cant_pages>=0)
      {


        page_list_t* search=temp->item.page_list_heap;
        while (search!=NULL)
        {
          if(search->page.number==page){
            if(cant_pages==0 && ult_page!=0){
              insert_freelist_pag(&search->page.free_list,0,ult_page);
              if(search->page.free_list->data.addr==0 && search->page.free_list->data.size==page_size){
                push_number(&temp->item.unpages,search->page.number);
                m_unset_owner(search->page.m_addr,search->page.m_addr+page_size-1);
                delete_page(&temp->item.page_list_heap,search->page.number);
              }
              return 0;
            }
            else if(cant_pages==0 && ult_page==0)return 0;
            if(search->prev!=NULL) page=search->prev->page.number;
            cant_pages--;
            push_number(&temp->item.unpages,search->page.number);
            m_unset_owner(search->page.m_addr,search->page.m_addr+page_size-1);
            delete_page(&temp->item.page_list_heap,search->page.number);
          }
          search=search->next;
        }
      }
      



    }


    temp=temp->next;


  }
  return 1;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){
    
    if(temp->item.process.pid==curr_pid){
      //si para pushear hay que crear una nueva pagina lo hace y pushea
      if(temp->item.page_list_stack==NULL || temp->item.stack_pointer==(addr_t)page_size-1){
        if(temp->item.page_count<max_pages){

          addr_t m_addr= less_freelist_pag(&m_free_list,page_size);
          m_set_owner(m_addr,m_addr+page_size-1);
          node_t* free_list=NULL;
          insert_freelist_pag(&free_list,1,page_size-1);
          int number=temp->item.unpages->data;
          delete_number(&temp->item.unpages,number);
          page_t page= {number,m_addr,page_size,free_list};
          push_page(&temp->item.page_list_stack,page);
          temp->item.page_count+=1;
          m_write(m_addr,val);
          temp->item.stack_pointer=0;
          out->addr=number*page_size;
          return 0;

        }
        return 1;
      }
      //si se va a pushear en una pagina existente se pushea
      addr_t m_addr= less_freelist_pag(&temp->item.page_list_stack->page.free_list,1);
      temp->item.stack_pointer+=1;
      m_write(temp->item.page_list_stack->page.m_addr + m_addr,val);
      out->addr= temp->item.page_list_stack->page.number*page_size + m_addr;
      return 0;
    }


    temp=temp->next;
  }
  return 1;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){

    if(temp->item.process.pid==curr_pid){
      //si para hacer pop no hay que eliminar una pagina lo hace
      if(temp->item.stack_pointer>0){
        insert_freelist_pag(&temp->item.page_list_stack->page.free_list,temp->item.stack_pointer,1);
        *out=m_read(temp->item.page_list_stack->page.m_addr + temp->item.stack_pointer);
        temp->item.stack_pointer-=1;
        return 0;
      }
      //en caso contrario la elimina y realiza el pop
      *out=m_read(temp->item.page_list_stack->page.m_addr + temp->item.stack_pointer);
      m_unset_owner(temp->item.page_list_stack->page.m_addr + temp->item.stack_pointer,temp->item.page_list_stack->page.m_addr + temp->item.stack_pointer+page_size-1);
      temp->item.stack_pointer=page_size-1;
      push_number(&temp->item.unpages,temp->item.page_list_stack->page.number);
      delete_page(&temp->item.page_list_stack,temp->item.page_list_stack->page.number);
      temp->item.page_count-=1;
      if(temp->item.page_list_stack==NULL) temp->item.stack_pointer=-1;
      return 0;
    }


    temp=temp->next;
  }
  return 1;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){

    if(temp->item.process.pid==curr_pid){
      //busca a que pagina pertenece la direccion y lee
      page_list_t* search=temp->item.page_list_heap;
      while (search!=NULL){
        if(search->page.number==(int)addr/page_size){
          *out=m_read(search->page.m_addr+addr%page_size);
          return 0;
        }

        search=search->next;
      }

    }
    temp=temp->next;
  }
  return 1;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
  //busca el proceso actual
  node_page_item_t* temp = ListadeProcesos;

  while (temp != NULL){

    if(temp->item.process.pid==curr_pid){
      //busca a que pagina pertenece la direccion y escribe
      page_list_t* search=temp->item.page_list_heap;
      while (search!=NULL){
        if(search->page.number==(int)addr/page_size){
          m_write(search->page.m_addr+addr%page_size,val);
          return 0;
        }

        search=search->next;
      }

    }
    temp=temp->next;
  }
  return 1;

}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  //busca el proceso actual
  int isNew=1;
  node_page_item_t* temp=ListadeProcesos;
  while(temp!=NULL){
    if(temp->item.process.pid == process.pid){
      isNew=0;
    }
    temp=temp->next;
  }
  curr_pid=process.pid;
  if(isNew == 0)return;
  //si el proceso no es nuevo lo inicializa y lo guarda
  page_list_t* page_list_code=NULL;
  page_list_t* page_list_heap=NULL;
  page_list_t* page_list_stack=NULL;
  int code_pages= process.program->size / page_size;
  int ult_page=process.program->size % page_size;
  if(ult_page !=0) code_pages+=1;
  int n=0;
  
  while (n<code_pages)
  {
    addr_t m_addr= less_freelist_pag(&m_free_list,page_size);
    m_set_owner(m_addr,m_addr+page_size-1);
    node_t* free_list=NULL;
    if(n==code_pages-1 && ult_page!=0){
      free_space_t new_data={ult_page ,page_size-ult_page};
      free_list = (node_t*) malloc(sizeof(node_t));
      free_list->data=new_data;
      
    }
    
    page_t page={n,m_addr,page_size,free_list};
    push_page(&page_list_code,page);
    n++;
  }
  

  number_list_t* unpages=NULL;
  n=max_pages-1;
  while (n>=code_pages)
  {
    push_number(&unpages,n);
    n--;
  }

  page_item_t item={code_pages,process,-1,page_list_code,page_list_heap,page_list_stack,unpages};
  push_page_item(&ListadeProcesos,item);
  return;
  
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  //busca el proceso actual
  node_page_item_t* temp=ListadeProcesos;
  while(temp!=NULL){
    if(temp->item.process.pid == process.pid){
      
      //elimina todas las listas de paginas del proceso
      page_list_t* search=temp->item.page_list_code;
      while (search!=NULL){
        m_unset_owner(search->page.m_addr,search->page.m_addr+page_size-1);

        search=search->next;
      }
      search=temp->item.page_list_heap;
      while (search!=NULL){
        m_unset_owner(search->page.m_addr,search->page.m_addr+page_size-1);

        search=search->next;
      }
      search=temp->item.page_list_stack;
      while (search!=NULL){
        m_unset_owner(search->page.m_addr,search->page.m_addr+page_size-1);

        search=search->next;
      }
      free(temp->item.page_list_code);
      free(temp->item.page_list_heap);
      free(temp->item.page_list_stack);
      free(temp->item.unpages);
      curr_pid=-1;
      delete_page_item(&ListadeProcesos,process.pid);
      return;
    }
    temp=temp->next;
  }


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Función para agregar un nodo al inicio de la lista
void beggining_freelist_pag(node_t** head, addr_t addr, int size) {

  free_space_t new_data={addr,size};
  node_t* new_node = (node_t*) malloc(sizeof(node_t));
  new_node->data = new_data;
  new_node->next = (*head);
  (*head)->prev = new_node;
  (*head) = new_node;
  
}

void insert_freelist_pag(node_t** head, addr_t addr, int size){
  
  
  if (*head ==NULL){
    node_t* new_node = (node_t*) malloc(sizeof(node_t));
    free_space_t new_data={addr,size};
    new_node->data = new_data;
    new_node->prev=NULL;
    new_node->next=NULL;
    
    (*head)= new_node;
    

    return;
  }
  node_t* temp = *head;
  node_t* predecesor = NULL;
  while(temp!= NULL ){
    if(temp->data.addr <= addr ){
      predecesor = temp;
      
    }
    temp= temp->next;
  }
  if(predecesor==NULL){
    beggining_freelist_pag(head,addr,size);
    return;
  }
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

addr_t less_freelist_pag(node_t** head, int size){
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

void push_page_item(node_page_item_t** head, page_item_t item){

  node_page_item_t* new_node= (node_page_item_t*)malloc(sizeof(node_page_item_t));
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

void delete_page_item(node_page_item_t** head, int pid){

  node_page_item_t* temp = *head;
  
  while (temp != NULL){
    if(temp->item.process.pid == pid){
      
      if(temp->prev != NULL){
      (temp->prev)->next= temp->next;
      }
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

void push_page(page_list_t** head, page_t page){
  page_list_t* new_node= (page_list_t*)malloc(sizeof(page_list_t));
  new_node->page= page;
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


void delete_page(page_list_t** head, int number){

  page_list_t* temp = *head;
  
  while (temp != NULL){
    if(temp->page.number == number){
      
      if(temp->prev != NULL){
      (temp->prev)->next= temp->next;
      }
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

void push_number(number_list_t** head,int number) {
  number_list_t* new_node = (number_list_t*) malloc(sizeof(number_list_t));
  new_node->data = number;
  new_node->next = (*head);
  if(*head==NULL) {

    (*head) = new_node;
    return;
  }
  (*head)->prev = new_node;
  (*head) = new_node;
}


void delete_number(number_list_t** head, int number){
  number_list_t* temp = *head;
  while (temp != NULL) {
    if(temp->data == number){
      
      if(temp->prev != NULL){
      (temp->prev)->next= temp->next;
      }
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
  return;
}