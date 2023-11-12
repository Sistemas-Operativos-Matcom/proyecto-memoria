#include <stdio.h>
#include <stdlib.h>
#include "bnbStruct.h"

// Inicializa la estructura bnbProcess
int g_heapsize = 512;

bnbProcess initProcess(int index) {    
    bnbProcess p;

    // Asigna memoria al heap
    p.heap = (int*)malloc(g_heapsize * sizeof(int));
    for (int i = 0; i<g_heapsize; i++)
    {
        p.heap[i] = 0;
    } 
    p.pid_proceso = -1;
    p.base = index * g_heapsize;    
    p.stack = g_heapsize -1;
    
    return p;
}

bnbProcess* init_bnbProcess_array(int proc_amount){
    // Asigna memoria para el array de bnbProcess
    bnbProcess* array_procesos = (bnbProcess*)malloc(proc_amount * sizeof(bnbProcess));

    for (int i = 0; i < proc_amount; i++) {
        array_procesos[i] = initProcess(i);
    }

    return array_procesos;
}


