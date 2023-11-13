#include <string.h>
#include <stdlib.h>

#include "stdio.h"
#include "bnb_manager.h"

typedef unsigned char byte;
typedef size_t addr_t;
#define MemAmountMax 1000000    
#define ProcAmountMax 1000   
#define PagAmountMax 100      
#define SizeOfPage 1000    
#define MaxStackSize 1000  
int curr_pid=0; int lastp=-1;
int spt[ProcAmountMax];
int from_pid[MemAmountMax];
int mem_alloc[MemAmountMax];
byte pag_memory[MemAmountMax];
addr_t sp[ProcAmountMax][MaxStackSize];

struct page_table {
    int mk[PagAmountMax][SizeOfPage];  // Multidimensional array to mark used positions in each page
    int pf[PagAmountMax];  // Array to store page frames
    int spent[PagAmountMax];        // Array to track used space in each page
} table[ProcAmountMax];

// Function to convert virtual page number and offset to address
size_t to_adress(int VPN,int offset){ 
  size_t ret=offset; VPN=(VPN<<10); 
  return ret|VPN;
}

int allocateMemory(size_t size, ptr_t *out) 
{
    int VPN = -1, offset = 0;

    for (int i = 0; i < PagAmountMax; i++) {
        if (SizeOfPage - table[i].spent[i] >= (int)size) {
            VPN = i;
            offset = table[i].spent[i];
            if (!table[i].spent[i]) table[i].pf[i] = 42;   
            table[i].spent[i] += size;
            break;
        }
    }
    if (VPN < 0)return 1;
    for (int i = offset; i < (int)(offset + size); i++) {
        table[i].mk[VPN][i] = 1;
        from_pid[table[i].pf[VPN] * SizeOfPage + i] = 42;   
    }
    out->addr = (addr_t)((VPN << 10) | offset);
    out->size = size;

    // Debug line to display the allocated memory
   // printf("[DEBUG] Allocated Memory: Address %lu, Size %zu\n", out->addr, out->size);

    return 0;
}

// Function to decode address into virtual page number and offset
void decode(size_t address, int *VPN, int *offset) 
{
    *offset = address & 0x3FF; *VPN = (address >> 10) & 0x3FFFFF;  
}

// Initialize the memory paging system
void m_pag_init(int argc, char **argv) 
{
    memset(mem_alloc, 0, sizeof(mem_alloc[0]) * MemAmountMax);
    for (int i = 0; i < MemAmountMax; i++) from_pid[i] = -1;
  
    memset(pag_memory, 0, sizeof(pag_memory[0]) * MemAmountMax);
    memset(spt, 0, sizeof(spt[0]) * ProcAmountMax);

    for (int i = 0; i < ProcAmountMax; i++)
    {
        memset(sp[i], 0, sizeof(sp[i][0]) * MaxStackSize);memset(table[i].pf, 0, sizeof(table[i].pf[0]) * PagAmountMax);
        memset(table[i].spent, 0, sizeof(table[i].spent[0]) * PagAmountMax);memset(table[i].mk, 0, sizeof(table[i].mk[0][0]) * PagAmountMax * SizeOfPage);
    }
}

// Allocate memory in the paging system
int m_pag_malloc(size_t size, ptr_t *out) 
{
    int VPN=-1,offset=0;
    for(int i=0;i<PagAmountMax;i++)
    {
      if(SizeOfPage-table[curr_pid].spent[i]>=(int)size){
        VPN=i; offset=table[curr_pid].spent[i];
        if(!table[curr_pid].spent[i]) table[curr_pid].pf[i]=++lastp;
        table[curr_pid].spent[i]+=size;
        break;
      }
    }
    if(VPN<0)return 1;
    for(int i=offset;i<(int)(offset+size);i++)
    {
      table[curr_pid].mk[VPN][i]=1;
      from_pid[table[curr_pid].pf[VPN]*SizeOfPage+i] = curr_pid;
    }
    out->addr=to_adress(VPN,offset);out->size=size;
    int pt=table[curr_pid].pf[VPN];
    int realp=pt*SizeOfPage+offset;
    // printf("[DEBUG] Allocation: Process %d, VPN %d, Offset %d, Real Position %d\n",curr_pid,VPN,offset,realp);
    return 0;
}

// Get the memory position for a given address
int mem_pos(addr_t addr)
{
    int VPN=0, offset=0;
    decode(addr,&VPN,&offset);
    int pt=table[curr_pid].pf[VPN];
    // printf("[DEBUG] Memory Position: Process %d, VPN %d, Offset %d, Position %d\n",curr_pid,VPN,offset,pt*SizeOfPage+offset);
    if(from_pid[pt*SizeOfPage+offset]!=curr_pid )return -1;
    return pt*SizeOfPage+offset;
}

// Store a value in the specified address
int m_pag_store(addr_t addr, byte val)
 {
    int pos=mem_pos(addr); if(pos<0)return 1;
    pag_memory[pos] = val;
    // printf("[DEBUG] Store: Process %d, Address %lu, Value %d\n",curr_pid,addr,val);
    return 0;
}

// Load a value from the specified address
int m_pag_load(addr_t addr, byte *out)
 {
    int pos=mem_pos(addr); if(pos<0)return 1;
    *out=pag_memory[pos];
    // printf("[DEBUG] Load: Process %d, Address %lu, Value %d\n",curr_pid,addr,*out);
    return 0;
}

// Update the current process on context switch
void m_pag_on_ctx_switch(process_t process)
{
    curr_pid = process.pid;
    // printf("[DEBUG] Context Switch: Process %d\n", curr_pid);
}

// Push a value onto the stack
int m_pag_push(byte val, ptr_t *out) 
{
    int ret=m_pag_malloc(1,out); if(ret)return 1;
    m_pag_store(out->addr,val);
    sp[curr_pid][spt[curr_pid]]=out->addr;
    spt[curr_pid]++;
    // printf("[DEBUG] Push: Process %d, Value %d, Address %lu\n", curr_pid, val, out->addr);
    return 0;
}

// Pop a value from the stack
int m_pag_pop(byte *out) {
    if(spt[curr_pid]==0)return 1;
    spt[curr_pid]--;
    addr_t addr=sp[curr_pid][spt[curr_pid]];
    int result = m_pag_load(addr,out);
    // printf("[DEBUG] Pop: Process %d, Value %d, Address %lu\n", curr_pid, *out, addr);
    return result;
}

// Clean up resources on process termination
void m_pag_on_end_process(process_t process) 
{
    int pid = process.pid;
    mem_alloc[pid]=pag_memory[pid]=spt[pid]=0;
    for(int i=0;i<PagAmountMax;i++) {
        if(table[pid].spent[i]) for(int j=0;j<SizeOfPage;j++) from_pid[table[pid].pf[i]*SizeOfPage+j]=-1;
    }
    memset(table[pid].pf, 0, sizeof(table[pid].pf[0]) * PagAmountMax);memset(sp[pid], 0, sizeof(sp[pid][0]) * MaxStackSize);
    memset(table[pid].mk, 0, sizeof(table[pid].mk[0][0]) * PagAmountMax * SizeOfPage);memset(table[pid].spent, 0, sizeof(table[pid].spent[0]) * PagAmountMax);
    // printf("[DEBUG] End Process: Process %d\n", pid);
}

// Free memory allocated for a pointer
int m_pag_free(ptr_t ptr) 
{
    m_pag_store(ptr.addr,0);
    int VPN=0, offset=0;
    decode(ptr.addr,&VPN,&offset);
    for(int i=offset;i<(int)(offset+ptr.size);i++)table[curr_pid].mk[VPN][i]=0;
    for(int i=table[curr_pid].spent[VPN];i>=0;i--)
    {
      if(table[curr_pid].mk[VPN][i])break;
      else table[curr_pid].spent[VPN]=i;
    }
    // printf("[DEBUG] Free: Process %d, Address %lu\n", curr_pid, ptr.addr);
    return 0;
}
