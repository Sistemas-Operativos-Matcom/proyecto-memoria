#ifndef BNBSTRUCT_H
#define BNBSTRUCT_H

typedef struct 
{   
    int pid_proceso;
    int code_size;
    int base;
    int* heap;
    int stack;
} bnbProcess;

bnbProcess* init_bnbProcess_array(int proc_amount);

#endif