#include "pag_manager.h"
#include "../utils.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>


typedef unsigned char byte;
typedef size_t addr_t;

#define max_mem 1000000  
#define max_pro 1000  
#define max_pag 100  
#define pag_size 1000
#define stack_size 1000  

byte pag_mem[max_mem];
int mem_alloc[max_mem];
int _pid[max_mem];
addr_t stack_pos[max_pro][stack_size];
int stk_point[max_pro];
int curr_pid=0;
int lastp=-1;

size_t to_adress(int vpn,int offset){ 
  size_t ret=0;
  ret=(ret|offset);
  vpn=(vpn<<10);
  ret=ret|vpn;
  return ret;
}

void decode(size_t address, int *vpn, int *offset) {
    *offset = address & 0x3FF;          
    *vpn = (address >> 10) & 0x3FFFFF;  
}

void m_pag_init(int argc, char **argv) {
    memset(mem_alloc, 0, sizeof(mem_alloc[0]) * max_mem);
    for (int i = 0; i < max_mem; i++) {
        _pid[i] = -1;
    }
    memset(pag_mem, 0, sizeof(pag_mem[0]) * max_mem);
    memset(stk_point, 0, sizeof(stk_point[0]) * max_pro);

    for (int i = 0; i < max_pro; i++) {
        memset(stack_pos[i], 0, sizeof(stack_pos[i][0]) * stack_size);
        memset(table[i].page_f, 0, sizeof(table[i].page_f[0]) * max_pag);
        memset(table[i].used, 0, sizeof(table[i].used[0]) * max_pag);
        memset(table[i].mk, 0, sizeof(table[i].mk[0][0]) * max_pag * pag_size);
    }
}

int m_pag_malloc(size_t size, ptr_t *out) {
    int vpn=-1,offset=0;
    for(int i=0 ; i<max_pag ; i++){
      if(pag_size-table[curr_pid].used[i]>=(int)size)
      {
        vpn=i; 
        offset=table[curr_pid].used[i];
        if(!table[curr_pid].used[i]) table[curr_pid].page_f[i]=++lastp;
        table[curr_pid].used[i]+=size;
        break;
      }
    }
    if(vpn<0)return 1;
    
    for(int i=offset;i<(int)(offset+size);i++){
      table[curr_pid].mk[vpn][i]=1;
      _pid[table[curr_pid].page_f[vpn]*pag_size+i] = curr_pid;
    }
    out->addr=to_adress(vpn,offset);
    out->size=size;

    int pt=table[curr_pid].page_f[vpn];
    int realp=pt*pag_size+offset;
    return 0;
}

int mem_pos(addr_t addr){
    int vpn=0, offset=0;
    decode(addr,&vpn,&offset);
    int pt=table[curr_pid].page_f[vpn];
    if(_pid[pt*pag_size+offset]!=curr_pid )return -1;
    return pt*pag_size+offset;
}

int m_pag_store(addr_t addr, byte val) {
    int pos=mem_pos(addr);
    if(pos<0)return 1;
    pag_mem[pos] = val;
    return 0;
}

int m_pag_load(addr_t addr, byte *out) {
    int pos=mem_pos(addr);
    if(pos<0)return 1;
    *out=pag_mem[pos];
    return 0;
}

void m_pag_on_ctx_switch(process_t process) {
    curr_pid = process.pid;
}

int m_pag_push(byte val, ptr_t *out) {
    int ret=m_pag_malloc(1,out);
    if(ret) return 1;
    m_pag_store(out->addr,val);
    stack_pos[curr_pid][stk_point[curr_pid]]=out->addr;
    stk_point[curr_pid]++;
    return 0;
}

int m_pag_pop(byte *out) {
    if(stk_point[curr_pid]==0)return 1;
    stk_point[curr_pid]--;
    addr_t addr=stack_pos[curr_pid][stk_point[curr_pid]];
    return m_pag_load(addr,out);
}

void m_pag_on_end_process(process_t process) {
    int pid = process.pid;
    mem_alloc[pid] = pag_mem[pid] = stk_point[pid] = 0;
    for(int i=0;i<max_pag;i++) {
        if(table[pid].used[i])
            for(int j=0;j<pag_size;j++){
                _pid[table[pid].page_f[i]*pag_size+j]=-1;
            }        
    }
    memset(stack_pos[pid], 0, sizeof(stack_pos[pid][0]) * stack_size);
    memset(table[pid].page_f, 0, sizeof(table[pid].page_f[0]) * max_pag);
    memset(table[pid].used, 0, sizeof(table[pid].used[0]) * max_pag);
    memset(table[pid].mk, 0, sizeof(table[pid].mk[0][0]) * max_pag * pag_size);
}


int m_pag_free(ptr_t ptr) {
    m_pag_store(ptr.addr,0);
    int vpn=0, offset=0;
    decode(ptr.addr,&vpn,&offset);

    for(int i=offset;i<(int)(offset+ptr.size);i++){
      table[curr_pid].mk[vpn][i]=0;
    }
    for(int i=table[curr_pid].used[vpn];i>=0;i--){
      if(table[curr_pid].mk[vpn][i])break;
      else table[curr_pid].used[vpn]=i;
    }
    return 0;
}