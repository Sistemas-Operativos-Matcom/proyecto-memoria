#include "pag_manager.h"
#include "../memory.h"
#include "stdio.h"

//a>>6
// ESTRUCTURAS

struct stack{
  int sp;
  int pagc;
  addr_t *pags;
};

struct vpn_pfn{
  int vpn;
  int pfn;
};

struct pagt{
  int n_pag;
  struct vpn_pfn *pt;
};

// INICIALIZACION
size_t pags=64;
int pagp=6;
int *pidl;
int *free_space;
size_t *heaps;
size_t *codes;
int *free_pages;
struct pagt *page_tables;
int pc=0; // proc counter
int cpid = -1;
int pags_am;
struct stack *stacks;
int proc_am;


// FUNC
size_t pag_of(size_t val){
  return val>>pagp;
}

//FUNC
int get_index_from_pid(int apid){
  for(int i=0;i<pc;i++){
    if(apid == pidl[i]){
      return i;
    }
  }
  // printf("\n ERROR get_index_from_pid no encontro el pid que recibio de argumento");
  return -1;
}

// FUNC
size_t offset_of(size_t addr){
  return addr - (pag_of(addr)<<pagp);
}

//FUNC
size_t get_free_page_addr(){
  // printf("\n entro get_free_page_addr\n");
  for(int i=0;i<pags_am;i++){
    if(free_pages[i] == 0){
      // printf("new page: %d", i);
      free_pages[i]=1;
      return i*pags;
    }
  }
  printf("\n get_free_page no encontro una pag libre");
  return -1;
}

  // struct pagt pages = page_tables[index];
  // printf("cant de pages: %d\n",pages.n_pag);
  // for(int i=0;i<pages.n_pag;i++){
  //   struct vpn_pfn e = pages.pt[i];
  //   printf("\n");
  //   printf("vpn: %d",e.vpn);
  //   printf("pfn: %d",e.pfn);
  //   printf("\n");
  // }
  // printf("va pag: %ld",va);
  // printf("\n");

size_t get_pa(addr_t addr){
  int index = get_index_from_pid(cpid);
  size_t va = codes[index] + addr;
  struct pagt table = page_tables[index];
  struct vpn_pfn *tuples = table.pt;

  for(int i=0;i<table.n_pag;i++){
    // printf("%ld",(size_t)tuples[i].vpn);
    // printf("\n");
    if((size_t)tuples[i].vpn == pag_of(va)){
      int pfn = tuples[i].pfn;
      return (pfn<<pagp) + offset_of(va);
    }
  }


  printf("Did not get the phisical address");
  printf("\n");
  return -1;
}


void add_vpn_pfn(int vpn, int pfn){
  int index = get_index_from_pid(cpid);


  // struct pagt pages = page_tables[index];
  // printf("cant de pages: %d\n",pages.n_pag);
  // for(int i=0;i<pages.n_pag;i++){
  //   struct vpn_pfn e = pages.pt[i];
  //   printf("\n");
  //   printf("vpn: %d",e.vpn);
  //   printf("pfn: %d",e.pfn);
  //   printf("\n");
  // }
  // // printf("va pag: %ld",va);
  // printf("\n");

  struct pagt *curr_pt = &page_tables[index];
  struct vpn_pfn row = {vpn,pfn};
  // printf("\n\n%d\n\n",curr_pt->n_pag);
  struct vpn_pfn e = curr_pt->pt[curr_pt->n_pag];
  // printf("\n\nMARKER\n\n");
  // printf("\n%d\n",e.vpn);
  // printf("\n%d\n",e.pfn);
  // printf("\n\nMARKER\n\n");

  curr_pt->pt[curr_pt->n_pag].vpn =vpn;
  curr_pt->pt[curr_pt->n_pag].pfn =pfn;
  curr_pt->n_pag = curr_pt->n_pag +1;
}

void occupy_mem(size_t s,size_t t){
  for(size_t i=s;i<t;i++){
    free_space[i]=1;
  }
}

size_t get_free_space(size_t need){
  int index = get_index_from_pid(cpid);
  struct pagt pages = page_tables[index];
  // printf("\nneed: %ld\n",need);
  for(int i=0;i<pages.n_pag;i++){
    struct vpn_pfn e = pages.pt[i];
    size_t s = e.pfn<<pagp;
    size_t t=s+pags;
    size_t c = need;
    while(s<heaps[index]&&s<t){
      // printf("\nfree_space[%ld]: %d",s,free_space[s]);
      if(free_space[s] == 0){
        c--;
        if(c==0){
          // printf("\nsalchichas: %ld\n",offset_of(s-need+1) + (e.vpn<<pagp));
          return offset_of(s-need+1) + (e.vpn<<pagp);}
      }
      else{c=need;}
      s++;
    }
  }
  return -1;
}

//FUNC
size_t get_new_pag(size_t size, int new_proc){
  // printf("\n\n Esta en el get_new_pag\n\n");
  int index = get_index_from_pid(cpid);
  size_t space_left =(pags-offset_of(heaps[index]));
  size_t addr;
  // printf("\n\nantes del if 1\n\n");
  if(size>space_left || new_proc==0){
    addr = get_free_page_addr();
    // printf("address: %ld",addr);
    m_set_owner(addr,addr+pags);
    occupy_mem(addr,addr+pags);
  }
  // printf("\n\nantes del if 2\n\n");
  if(new_proc==0 && size>space_left){
    add_vpn_pfn(pag_of(heaps[index]),pag_of(addr));
    // printf("\n\ncarambolas\n\n");
    size_t ans = get_new_pag(size-space_left,1) + pags;
    heaps[index] = ans;
    addr = get_free_page_addr();
    add_vpn_pfn(pag_of(heaps[index]),pag_of(addr));
    m_set_owner(addr,addr+pags);
    occupy_mem(addr,addr+pags);
    return ans;
  }
  // printf("\n\nantes del if 3\n\n");
  if(size>space_left && new_proc==1){
    heaps[index] = (pag_of(heaps[index])+1)*pags;
    add_vpn_pfn(pag_of(heaps[index]),pag_of(addr));
    return get_new_pag(size-space_left, 1);
  }
  // printf("\n\nantes del if 4\n\n");
  if(new_proc==0 && size<space_left){
    // if(size == 0){size=1;}
    // printf("\nvpn: %ld\n",pag_of(heaps[index]));printf("\npfn: %ld\n",pag_of(addr));
    heaps[index] = size;
    add_vpn_pfn(pag_of(heaps[index]),pag_of(addr));
    return heaps[index];
  }
  heaps[index] =heaps[index] + size;
  // printf("\n\nantes del return\n\n");
  return heaps[index];
}
/*
addr_t search_fit(size_t size){
  int index = get_index_from_pid(cpid);
  for(int i=0;i<;i++){
    if(free_space[i]==1)
  }
}*/

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) {
  // printf("\n\nargc: %d\n\n",argc);
  pags_am = m_size()/pags;
  free_pages = calloc(pags_am,sizeof(int));
  free_space = calloc(m_size(),sizeof(int));
  heaps = calloc(1,sizeof(size_t));
  codes = calloc(1,sizeof(size_t));
  page_tables = malloc(sizeof(struct pagt)*1);
  stacks = calloc(1,sizeof(struct stack));
  pidl = malloc(sizeof(int)*1);
  pc=0;
  cpid=-1;
  proc_am=0;
    struct vpn_pfn *pt = malloc(sizeof(struct vpn_pfn)*pags_am);
    struct pagt page_table = {0, pt};
    page_tables[proc_am] = page_table;
    addr_t *p = malloc(pags_am*sizeof(addr_t));
    struct stack s = {0,0,p};
    stacks[proc_am] = s;
  // printf("\n\nTermino init\n\n");
  // exit(1);
}


// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) {
  int index = get_index_from_pid(cpid);
  // printf("\nindex: %d")
  size_t space = get_free_space(size);

  if(space==(size_t)-1)
  {size_t c_heap = heaps[index];
  size_t new_heap = heaps[index] + size;
  out->addr = heaps[index]-codes[index];
  if(pag_of(c_heap) != pag_of(new_heap)){
    printf("\n\nentered if in malloc\n\n");
    size_t size_need = size - offset_of(c_heap);
    new_heap = get_new_pag(size_need,1);
  }
  heaps[index]=new_heap;
  return 0;

  }
  out->addr = space;
  // printf("\nsalchichas\n");
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) {
  addr_t s = get_pa(ptr.addr);
  size_t t = s+ptr.size;
  for(size_t i=s;i<t;i++){
    free_space[i]=0;
  }
  return 0;
}

// Agrega un elemento al stack CHECAR QUE PINCHA
int m_pag_push(byte val, ptr_t *out) {
  int index = get_index_from_pid(cpid);
  struct stack *curr_stack = &stacks[index];
  if(curr_stack->pagc == 0 || curr_stack->sp+1==(int)pags){
    if(curr_stack->sp+1==(int)pags){
      addr_t a = curr_stack->sp + curr_stack->pags[curr_stack->pagc-1];
      m_write(a,val);
      out->addr = a;
      addr_t addr = get_free_page_addr();
      curr_stack->pags[curr_stack->pagc] = addr;
      curr_stack->pagc++;
      curr_stack->sp=0;
      m_set_owner(addr,addr+64);
    }
    else if(curr_stack->pagc == 0 ){
      addr_t addr = get_free_page_addr();
      curr_stack->pags[curr_stack->pagc] = addr;
      curr_stack->pagc++;
      curr_stack->sp=0;
      addr_t a = curr_stack->sp + curr_stack->pags[curr_stack->pagc-1];
      m_set_owner(addr,addr+64);
      m_write(a,val);
      curr_stack->sp++;
      out->addr = a;
    }
    return 0;
  }
  addr_t a = curr_stack->sp + curr_stack->pags[curr_stack->pagc-1];
  m_write(a,val);
  out->addr = a;
  curr_stack->sp++;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out) {
    int index = get_index_from_pid(cpid);
    struct stack *curr_stack = &stacks[index];
    curr_stack->sp = curr_stack->sp-1;
    if(curr_stack->sp < 0){
      curr_stack->pagc = curr_stack->pagc-1;
      if(curr_stack->pagc==0){
        printf("\nERROR: Cannot pop, stack is empty\n");
        return 1;
      }
      curr_stack->sp=pags-1;
    }
    addr_t a = curr_stack->sp + curr_stack->pags[curr_stack->pagc-1];
    byte ans =m_read(a);
    *out = ans;
    return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) {

    addr = get_pa(addr);
    // printf("\n pa: %ld \n",addr);
    byte ans = m_read(addr);
    // printf("\n m_read: %d \n",ans);
    *out = ans;
    // printf("\n *out: %d \n",*out);

    return 0;
}

// Almacena un valor en una dirección determinada ASUMO QUE NUNCA ME VAN A PEDIR UNA MEM NO RESERVADA EN LOS CASOS DE PRUEBA
int m_pag_store(addr_t addr, byte val) {

  // printf("addr asked: %ld", addr);
  // printf("\n");

  addr_t ans = get_pa(addr);

  // printf("phisical addr: %ld", ans);
  // printf("\n");

  // printf("val: %d \n",val);

  m_write(ans,val);

  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) {
  // printf("\n%d\n",process.pid);
  int index = get_index_from_pid(process.pid);
  if(index==-1)
  {if(proc_am!=0)
  {proc_am++;
  // printf("\n\nproc_am: %d\n\n",proc_am);
  heaps=(size_t*)realloc(heaps,sizeof(size_t)*proc_am);
  codes=(size_t*)realloc(codes,sizeof(size_t)*proc_am);
  page_tables=(struct pagt*)realloc(page_tables,sizeof(struct pagt)*proc_am);
  stacks=(struct stack*)realloc(stacks,sizeof(struct stack)*proc_am);
  pidl=(int*)realloc(pidl,sizeof(int)*proc_am);

  struct vpn_pfn *pt = malloc(sizeof(struct vpn_pfn)*pags_am);
  struct pagt page_table = {0, pt};
  page_tables[proc_am-1] = page_table;
  addr_t *p = malloc(pags_am*sizeof(addr_t));
  struct stack s = {0,0,p};
  stacks[proc_am-1] = s;}
  else{proc_am=1;}}

  size_t code_s = process.program->size;
  // printf("\n%ld\n",process.program->size);
  cpid = process.pid;
  for(int i=0;i<pc;i++){
    // printf("\n\nEsta en el for loop\n\n");
    if(process.pid == pidl[i]){
      return;
    }
  }
  pidl[pc] = process.pid;
  codes[pc]= code_s;
  pc = pc +1;
  // printf("\n\nllego hasta antes de get_new_pag\n\n");
  get_new_pag(process.program->size,0);
    // printf("\n\nllego hasta antes de get_new_pag\n\n");

  // exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) {
  int index = get_index_from_pid(process.pid);
  // printf("\n index: %d \n",index);
  struct stack *cs = &stacks[index];
  struct pagt *page_table = &page_tables[index];
  struct vpn_pfn *table = page_table->pt;
  for(int i=0;i<page_table->n_pag;i++){
    struct vpn_pfn *curr = &table[i];
    addr_t a = curr->pfn<<pagp;
    m_unset_owner(a,a+pags);
    // printf("\n vpn: %d pfn: %d \n",curr.vpn,curr.pfn);
  }
  for(int i=0;i<cs->pagc;i++){
    // printf("\n%d: %ld\n",i,cs->pags[i]);
    m_unset_owner(cs->pags[i],cs->pags[i]+pags);
  }
  cs->sp=0;
  cs->pagc=0;
  page_table->n_pag=0;
}
