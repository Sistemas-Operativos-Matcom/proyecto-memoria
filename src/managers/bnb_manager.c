#include "bnb_manager.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define Max_Pid 32
#define HeapSize 100
#define StackSize 150

int ProgCnt;
int Prog_Size;

char Alloc[100000];
int UsedSlot[32];
int MemSlot[Max_Pid];
int StackPointer[32];
int HeapPointer[32];
int HeapBase[32];
int StackBase[32];
int curpid;


// Esta función se llama cuado se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
 
  memset(Alloc,0,sizeof(Alloc));
  ProgCnt=m_size()/512;
  if(ProgCnt>32){
    ProgCnt=32;
  }
  Prog_Size=m_size()/ProgCnt;
  memset(MemSlot,-1,sizeof(MemSlot));
  memset(UsedSlot,-1,sizeof(UsedSlot));
  curpid=-1;
  int last=0;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    if(curpid==-1){
      return 1;
    }
    int last=-1;
    for(int i=HeapPointer[MemSlot[curpid]];i<HeapBase[MemSlot[curpid]]+HeapSize;i++){
        if(Alloc[i]==1){
          last=-1;
          continue;
        }
        if(last==-1){
          last=i;
        }
        if(i-last+1==(int)size){
          for(int j=last;j<=i;j++){Alloc[j]=1;}
          out->addr=last;
          HeapPointer[MemSlot[curpid]]=i+1;
          out->size=size;
          m_set_owner(out->addr,out->addr+out->size-1);
          return 0;
        }
    }
    HeapPointer[MemSlot[curpid]]=HeapBase[MemSlot[curpid]];
    last=-1;
    for(int i=HeapPointer[MemSlot[curpid]];i<HeapBase[MemSlot[curpid]]+HeapSize;i++){
        if(Alloc[i]==1){
          last=-1;
          continue;
        }
        if(last==-1){
          last=i;
        }
        if(i-last+1==(int)size){
          for(int j=last;j<=i;j++){Alloc[j]=1;}
          out->addr=last;
          HeapPointer[MemSlot[curpid]]=i+1;
          out->size=size;
          m_set_owner(out->addr,out->addr+out->size-1);
          return 0;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  if(curpid==-1){
      return 1;
  }
  if((int)ptr.addr<HeapBase[MemSlot[curpid]] || (int)(ptr.addr+ptr.size)>HeapBase[MemSlot[curpid]]+HeapSize){
      return 1;
  }
  for(int i=ptr.addr;i<(int)(ptr.addr+ptr.size);i++)if(!Alloc[i])return 1;
  m_unset_owner(ptr.addr,ptr.addr+ptr.size);
  for(int i=ptr.addr;i<(int)(ptr.addr+ptr.size);i++)Alloc[i]=0;
  return 0;
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  if(curpid==-1){
    return 1;
  }
  if(StackPointer[MemSlot[curpid]]==StackBase[MemSlot[curpid]]-StackSize){
    return 1;
  }
  m_write(StackPointer[MemSlot[curpid]],val);
  StackPointer[MemSlot[curpid]]--;
  out->addr=StackPointer[MemSlot[curpid]];
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out) {
  if(curpid==-1){
      return 1;
  }
  if(StackPointer[MemSlot[curpid]]==StackBase[MemSlot[curpid]]){
     return 1;
  }
  StackPointer[MemSlot[curpid]]++;
  *out=m_read(StackPointer[MemSlot[curpid]]);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if(curpid==-1){
      return 1;
  }
  if((int)addr<HeapBase[MemSlot[curpid]] || (int)addr>=HeapBase[MemSlot[curpid]]+HeapSize){
      return 1;
  }
  *out=m_read(addr); 
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  if(curpid==-1){
      return 1;
  }
  if((int)addr<HeapBase[MemSlot[curpid]] || (int)addr>=HeapBase[MemSlot[curpid]]+HeapSize){
      return 1;
  }
  m_write(addr,val);
   return 0; 
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
 
  curpid=process.pid;
  if(MemSlot[curpid]==-1){
    for(int i=0;i<ProgCnt;i++){
      if(UsedSlot[i]==-1){
        if(process.program->size==0){
          printf("PROFEEE el program->size no deberia ser 0!!!\n");
          process.program->size=1;
        }
        UsedSlot[i]=curpid;
        MemSlot[curpid]=i;
        StackPointer[i]=(i+1)*Prog_Size-1;
        HeapPointer[i]=i*Prog_Size+process.program->size;
        HeapBase[i]=HeapPointer[i];
        StackBase[i]=StackPointer[i];
        m_set_owner(i*Prog_Size,i*Prog_Size+process.program->size-1);
        m_set_owner((i+1)*Prog_Size-StackSize,(i+1)*Prog_Size-1);
        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
    if(MemSlot[process.pid]==-1){
      fprintf(stderr, "No Such Process\n");
       exit(1);
    } 
    UsedSlot[MemSlot[process.pid]]=-1;
    for(int i=MemSlot[process.pid]*Prog_Size;i<(MemSlot[process.pid]+1)*Prog_Size;i++){
        if(Alloc[i]){
          m_unset_owner(i,i);
        }
        Alloc[i]=0;
    }
    m_unset_owner(MemSlot[process.pid]*Prog_Size,MemSlot[process.pid]*Prog_Size+process.program->size-1);
    m_unset_owner((MemSlot[process.pid]+1)*Prog_Size-StackSize,(MemSlot[process.pid]+1)*Prog_Size-1);
    
    MemSlot[process.pid]=-1;
}
