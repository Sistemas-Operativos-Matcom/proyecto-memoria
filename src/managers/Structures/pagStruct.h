#ifndef PAGSTRUCT_H
#define PAGSTRUCT_H

#include "stack.h"

typedef struct 
{   
    int proc_pid;
    int pages[4];
    int data[4][512];
    stack* proc_stack;
} pagStruct;

pagStruct* init_pagProcs_array(int size);

#endif