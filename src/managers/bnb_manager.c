

#include <string.h>
#include <stdlib.h>

#include "stdio.h"
#include "bnb_manager.h"

typedef struct {
    size_t bse;
    size_t bnd;
} bnb_info_t;
typedef unsigned char byte;
typedef size_t addr_t;

// Globalllllllll  
#define ProcAmountMax  1000
#define MemAmountMax 2000000  
#define SpaceAmountMax 1000
#define MaxStackSize 2000  
byte st[ProcAmountMax][MaxStackSize];
byte mem[MemAmountMax];
byte alloc[MemAmountMax];
size_t st_top[ProcAmountMax];
bnb_info_t mem_proc_data[ProcAmountMax];
int fin = 0;
int curpid = 0;
bnb_info_t current_process_bnb;

// initializes the array of process memory information
void mem_proc_data_init()
{
    for (int i = 0; i < ProcAmountMax; i++) mem_proc_data[i].bse =mem_proc_data[i].bnd = 0; 
}

  // initializes the arrays of memory
void initialize_memory()
{
    memset(alloc, 0, MemAmountMax);memset(mem, 0, MemAmountMax);memset(st_top, 0, MaxStackSize);
    current_process_bnb.bse = current_process_bnb.bnd = 0;
}

  // initialization function for memory and information of process memory
void m_bnb_init(int argc, char **argv)
{
    initialize_memory();mem_proc_data_init();
}

  // utility function to find the maximum of two integers
int chbigger(int a, int b) {
    return (a > b ? a : b);
}

  // allocates memory for a process
int m_bnb_malloc(size_t size, ptr_t *out)
{
    if (current_process_bnb.bse + current_process_bnb.bnd + size > MemAmountMax) return 1; // Memory allocation error
    
      // checks for memory overlap
    for (int i = current_process_bnb.bse + current_process_bnb.bnd; i < (int)(current_process_bnb.bse + current_process_bnb.bnd + size); i++) {
        if (alloc[i]) return 1; // Memory overlap error
        alloc[i] = 1;
    }
    // allocates memory and updates the information of the process
    out->addr = current_process_bnb.bse + current_process_bnb.bnd;
    out->size = size;
    current_process_bnb.bnd += size;
    fin = chbigger(fin,current_process_bnb.bse + current_process_bnb.bnd);
    
    return 0; // Success
}

//     frees memory for a process
int m_bnb_free(ptr_t ptr)
{
    for (int i = current_process_bnb.bse + ptr.addr; i < (int)(current_process_bnb.bse + ptr.addr + ptr.size); i++)
    {
        if (!alloc[i]) return 1; //   invalid release operation
        alloc[i] = 0;
    }
    return 0; // Success
}

//pushes a byte onto the stack
int m_bnb_push(byte val, ptr_t *out)
{
    int top = st_top[curpid];
    if (top >= MaxStackSize) return 1; // Stack overflow
    st[curpid][top] = val;
    out->addr = top;out->size = sizeof(byte);
    st_top[curpid]++;
    return 0; // Success
}

// pops a byte from the stack
int m_bnb_pop(byte *out)
{
    // Stack underflow
    if (st_top[curpid] == 0) return 1; 
    st_top[curpid]--;
    *out = st[curpid][st_top[curpid]];
    return 0; // Success
}

//     loads a byte from memory
int m_bnb_load(addr_t addr, byte *out)
{
    if (addr < current_process_bnb.bse||addr >= current_process_bnb.bse + current_process_bnb.bnd) return 1; // Invalid memory access
    *out = mem[addr];
    return 0; // Success
}

  //      stores a byte in memory
int m_bnb_store(addr_t addr, byte val)
{
    if (addr < current_process_bnb.bse || addr >= current_process_bnb.bse+current_process_bnb.bnd) return 1; // Invalid memory access
    mem[addr] = val;
    return 0; // Success
}

// handles the context switch between processes
void m_bnb_on_ctx_switch(process_t process)
{
      // saves the information of the current process
    mem_proc_data[curpid]=current_process_bnb;
    // initializes memory for a new process if necessary
    if(process.pid!=0&&mem_proc_data[process.pid].bse==0)mem_proc_data[process.pid].bse=fin+SpaceAmountMax;
      // sets the information of the current process to the new process
    current_process_bnb=mem_proc_data[process.pid];
    curpid=process.pid;
}

// handles the end of a process
void m_bnb_on_end_process(process_t process)
{
      // frees the memory allocated for the terminating process
    for(int i=mem_proc_data[process.pid].bse; i < (int)(mem_proc_data[process.pid].bse+mem_proc_data[process.pid].bnd); i++)alloc[i] = 0;
    // updates the position of the last used memory
    if (fin==(int)(mem_proc_data[process.pid].bse+mem_proc_data[process.pid].bnd))fin=mem_proc_data[process.pid].bse;
      // resets the information of the process memory
    mem_proc_data[process.pid].bse=mem_proc_data[process.pid].bnd=0;
}
