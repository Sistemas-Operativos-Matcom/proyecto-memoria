#include "pag_manager.h"

#include "stdio.h"
#include "string.h"

#define Max_Pid 32
#define PageSize 8
#define HeapSize 36
#define StackSize 60

int PagProgCnt;
int PagProg_Size;
int PageCount;
int PagMemSlot[Max_Pid];
int PagUsedSlot[Max_Pid];
unsigned int PTE[Max_Pid][1024];
struct {
  int Hp,Sp,Hb,Sb;
} Tab[Max_Pid];


int Pagcurpid;
// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  PagProgCnt=m_size()/1024;
  if(PagProgCnt>32){
    PagProgCnt=32;
  }
  PagProg_Size=m_size()/PagProgCnt;
  PageCount=PagProg_Size/PageSize;
  memset(PagMemSlot,-1,sizeof(PagMemSlot));
  memset(PagUsedSlot,-1,sizeof(PagUsedSlot));
  for(int i=0;i<PagProgCnt;i++){
    for(int j=0;j<PageCount;j++){
      PTE[i][j]=i*PageCount+j;
    }
  }
  Pagcurpid=-1;
  int last=0;
}

void AllocPage(int VPN,int Writtable,int Stack){
    int hardaddr=PTE[PagMemSlot[Pagcurpid]][VPN]&((1<<20)-1);
    PTE[PagMemSlot[Pagcurpid]][VPN]|=(1<<20);
    if(Writtable){
      PTE[PagMemSlot[Pagcurpid]][VPN]|=(1<<21);
    }
    if(Stack){
      PTE[PagMemSlot[Pagcurpid]][VPN]|=(1<<22);
    }
    m_set_owner(hardaddr*8,hardaddr*8+7);  
}

void FreePage(int VPN){
    int hardaddr=PTE[PagMemSlot[Pagcurpid]][VPN]&((1<<20)-1);
    PTE[PagMemSlot[Pagcurpid]][VPN]&=(~(1<<20));
      PTE[PagMemSlot[Pagcurpid]][VPN]&=(~(1<<21));
      PTE[PagMemSlot[Pagcurpid]][VPN]&=(~(1<<22));
    m_unset_owner(hardaddr*8,hardaddr*8+7);  
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
    int last=-1;
    for(int i=Tab[PagMemSlot[Pagcurpid]].Hp;i<HeapSize;i++){
        if(PTE[PagMemSlot[Pagcurpid]][i]&(1<<20)){
           last=-1;
           continue; 
        }
        if(last==-1)last=i;
        if((i-last+1)*8>=(int)size){
            out->addr=last*8;
            out->size=size;
            for(;last<=i;last++){
                AllocPage(last,1,0);
            }
            return 0;
        }
    }
    Tab[PagMemSlot[Pagcurpid]].Hp=Tab[PagMemSlot[Pagcurpid]].Hb;
    last=-1;
    for(int i=Tab[PagMemSlot[Pagcurpid]].Hp;i<HeapSize;i++){
        if(PTE[PagMemSlot[Pagcurpid]][i]&(1<<20)){
           last=-1;
           continue; 
        }
        if(last==-1)last=1;
         if((i-last+1)*8>=(int)size){
            out->addr=last*8;
            out->size=size;
            for(;last<=i;last++){
                AllocPage(last,1,0);
            }
            return 0;
        }
    }
    return 1;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  int VPN=ptr.addr/8;
  if(VPN<Tab[PagMemSlot[Pagcurpid]].Hb || (int)(ptr.addr+ptr.size-1+7)/8>=Tab[PagMemSlot[Pagcurpid]].Hb+HeapSize){
    return 1;
  }
  for(int i=ptr.addr/8;i<(int)(ptr.addr+ptr.size-1+7)/8;i++){
    FreePage(i);
  }
  return 0;
}


int decode(int vaddr){
  int VPN=vaddr/8;
  int hardaddr=PTE[PagMemSlot[Pagcurpid]][VPN]&((1<<20)-1);
  return hardaddr*8+(vaddr&7);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) {
  if(Tab[PagMemSlot[Pagcurpid]].Sp==Tab[PagMemSlot[Pagcurpid]].Sb-StackSize*8){
    return 1;
  }
  m_write(decode(Tab[PagMemSlot[Pagcurpid]].Sp),val);
  out->addr=Tab[PagMemSlot[Pagcurpid]].Sp;
  Tab[PagMemSlot[Pagcurpid]].Sp--;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
   if(Tab[PagMemSlot[Pagcurpid]].Sp==Tab[PagMemSlot[Pagcurpid]].Sb-StackSize*8){
    return 1;
  }
  Tab[PagMemSlot[Pagcurpid]].Sp++;
  *out=m_read(decode(Tab[PagMemSlot[Pagcurpid]].Sp));
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {
    int VPN=addr/8;
    if(VPN<Tab[PagMemSlot[Pagcurpid]].Hb || VPN>=Tab[PagMemSlot[Pagcurpid]].Hb+HeapSize){
    return 1;
    }
    *out=m_read(decode(addr));
    return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) {
    int VPN=addr/8;
    if(VPN<Tab[PagMemSlot[Pagcurpid]].Hb || VPN>=Tab[PagMemSlot[Pagcurpid]].Hb+HeapSize){
    return 1;
    }
    m_write(decode((int)addr),val);
    return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  Pagcurpid=process.pid;
  if(PagMemSlot[Pagcurpid]==-1){
    for(int i=0;i<PagProgCnt;i++){
      if(PagUsedSlot[i]==-1){
        if(process.program->size==0){
          printf("PROFEEE el program->size no deberia ser 0!!!\n");
          process.program->size=1;
        }
        PagUsedSlot[i]=Pagcurpid;
        PagMemSlot[Pagcurpid]=i;
        Tab[i].Sp=PagProg_Size-1;
        Tab[i].Hp=(process.program->size-1)/8+1;
        Tab[i].Hb=Tab[i].Hp;
        Tab[i].Sb=Tab[i].Sp;
        for(int j=0;j<(int)(process.program->size-1)/8+1;j++){
            AllocPage(j,0,0);
        }
        for(int j=PagProg_Size/8-1;j>=PagProg_Size/8-StackSize;j--){
            AllocPage(j,1,1);
        }
        break;
      }
    }
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
    if(PagMemSlot[process.pid]==-1){
      fprintf(stderr, "No Such Process\n");
       exit(1);
    } 
    PagUsedSlot[PagMemSlot[process.pid]]=-1;

    for(int i=Tab[PagMemSlot[process.pid]].Hb;i<Tab[PagMemSlot[process.pid]].Hb+HeapSize;i++){
      if(PTE[PagMemSlot[process.pid]][i]&(1<<20)){
        FreePage(i);
      }
    }

    for(int j=0;j<(int)(process.program->size-1)/8+1;j++){
        FreePage(j);
    }
    for(int j=PagProg_Size/8-1;j>=PagProg_Size/8-StackSize;j--){
        FreePage(j);
    }
    
    PagMemSlot[process.pid]=-1;
}
