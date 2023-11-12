#include "bnb_manager.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef size_t addr_t;

#define MAX_MEMORY 2000000  
#define MAX_PROCESSES 1000
#define STACK_SIZE 2000  
#define SPACE 1000

byte stack[MAX_PROCESSES][STACK_SIZE];
byte memory[MAX_MEMORY];
byte alloc[MAX_MEMORY];
size_t stack_top[MAX_PROCESSES];


typedef struct {
    size_t base;
    size_t bounds;
} bnb_info_t;

bnb_info_t process_memory_info[MAX_PROCESSES];
bnb_info_t current_process_bnb;
int current_pid=0;
int lastu=0;


int max(int a,int b){
  if(a>b)return a; else return b;
}

void initialize_process_memory_info() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_memory_info[i].base = 0;
        process_memory_info[i].bounds = 0;
    }
}

void initialize_memory() {
    memset(memory, 0, MAX_MEMORY);
    memset(alloc, 0, MAX_MEMORY);
    memset(stack_top, 0, STACK_SIZE);
    current_process_bnb.base=0;
    current_process_bnb.bounds=0;
}

void m_bnb_init(int argc, char **argv) {
    initialize_memory();
    initialize_process_memory_info();
}

int m_bnb_malloc(size_t size, ptr_t *out) {
    if (current_process_bnb.base + current_process_bnb.bounds + size > MAX_MEMORY) {
        return 1;
    }

    for(int i=current_process_bnb.base + current_process_bnb.bounds;i<(int)(current_process_bnb.base+current_process_bnb.bounds+size);i++){
        if(alloc[i])return 1; // Ya habia algo en esa posicion de memoria
        alloc[i]=1;
    }
    out->addr = current_process_bnb.base + current_process_bnb.bounds;
    out->size = size;
    current_process_bnb.bounds += size;
    lastu=max(current_process_bnb.base+current_process_bnb.bounds , lastu );

    return 0;
}


int m_bnb_free(ptr_t ptr) {
    for(int i=current_process_bnb.base + ptr.addr ;i<(int)(current_process_bnb.base+ptr.addr+ptr.size);i++){
        if(!alloc[i])return 1;
        alloc[i]=0;
    }
    return 0;
}

int m_bnb_push(byte val, ptr_t *out) {
    int top=stack_top[current_pid];

    if (top >= STACK_SIZE) {
        return 1; 
    }
    stack[current_pid][top] = val;
    out->addr = top;
    out->size = sizeof(byte);
    stack_top[current_pid]++;
    return 0;
}

int m_bnb_pop(byte *out) {
    if (stack_top[current_pid]== 0) {
        return 1;
    }
    stack_top[current_pid]--;
    *out = stack[current_pid][stack_top[current_pid]];
    return 0;
}

int m_bnb_load(addr_t addr, byte *out) {
    if (addr < current_process_bnb.base || addr >= current_process_bnb.base + current_process_bnb.bounds) {
        return 1;
    }
    *out = memory[addr];
    return 0;
}

int m_bnb_store(addr_t addr, byte val) {
    if (addr < current_process_bnb.base || addr >= current_process_bnb.base + current_process_bnb.bounds) {
        return 1;
    }
    memory[addr] = val;
    return 0;
}

void m_bnb_on_ctx_switch(process_t process) {
    process_memory_info[current_pid] = current_process_bnb;
    
    if(process.pid!=0 && process_memory_info[process.pid].base==0 )
       process_memory_info[process.pid].base=lastu+SPACE;
    current_process_bnb = process_memory_info[process.pid];
    current_pid = process.pid;
}

void m_bnb_on_end_process(process_t process) {
    for(int i=process_memory_info[process.pid].base  ;
        i<(int)(process_memory_info[process.pid].base + process_memory_info[process.pid].bounds);i++){
        alloc[i]=0;
    }
    if(lastu==(int)(process_memory_info[process.pid].base + process_memory_info[process.pid].bounds )){
        lastu=process_memory_info[process.pid].base;
    }
    process_memory_info[process.pid].base=0;
    process_memory_info[process.pid].bounds=0;
}