#include "bnb_manager.h"
#include "stdio.h"

#define KB_SIZE(x) x*1024
#define Prog_Size 16
#define MaxProgCnt 20
#define Max_Pid 500000

char Mem[KB_SIZE(MaxProgCnt*Prog_Size)];

int UsedSlot[MaxProgCnt];
int MemSlot[Max_Pid];
int StackPointer[MaxProgCnt];
int HeapPointer[MaxProgCnt];
int HeapBase[MaxProgCnt];
int StackBase[MaxProgCnt];
int curpid;


// Esta función se llama cuado se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv) {
  memset(Mem,0,sizeof(Mem));
  memset(MemSlot,-1,sizeof(MemSlot));
  memset(UsedSlot,-1,sizeof(UsedSlot));
  curpid=-1;
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out) {
    if(curpid==-1){
      return 1;
    }
    out->size=size;
    out->addr=HeapPointer[MemSlot[curpid]];
    HeapPointer[MemSlot[curpid]]+=size;
    return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr) {
  if(curpid==-1){
      return 1;
  }
  

  return 0;

}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out) {
  if(curpid==-1){
    return 1;
  }
  if(StackPointer[MemSlot[curpid]]==HeapPointer[MemSlot[curpid]]){
    return 1;
  }
  Mem[StackPointer[MemSlot[curpid]]]=val;
  out->addr=StackPointer[MemSlot[curpid]]-HeapBase[MemSlot[curpid]];
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
  StackPointer[MemSlot[curpid]]--;
  *out=Mem[StackPointer[MemSlot[curpid]]];
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out) {
  if(curpid==-1){
      return 1;
  }
  if(addr<0 || HeapBase[MemSlot[curpid]]+addr>=HeapPointer[MemSlot[curpid]]){
      return 1;
  }
  *out=Mem[HeapBase[MemSlot[curpid]]+addr]; 
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val) {
  if(curpid==-1){
      return 1;
  }
  if(addr<0 || HeapBase[MemSlot[curpid]]+addr>=HeapPointer[MemSlot[curpid]]){
      return 1;
  }
  Mem[HeapBase[MemSlot[curpid]]+addr]=val; 
   return 0; 
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process) {
  curpid=process.pid;
  if(MemSlot[curpid]==-1){
    for(int i=0;i<MaxProgCnt;i++){
      if(UsedSlot[i]==-1){
        UsedSlot[i]=curpid;
        MemSlot[curpid]=i;
        StackPointer[i]=(i+1)*KB_SIZE(Prog_Size)-1;
        HeapPointer[i]=i*KB_SIZE(Prog_Size)+process.program->size;
        HeapBase[i]=HeapPointer[i];
        StackBase[i]=StackPointer[i];
        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process) {
    if(MemSlot[curpid]==-1){
      fprintf(stderr, "No Such Process\n");
       exit(1);
    } 
    UsedSlot[MemSlot[curpid]]=-1;
    MemSlot[curpid]=-1;
}
