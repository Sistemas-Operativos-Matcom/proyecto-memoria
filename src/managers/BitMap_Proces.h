#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BitMap.h"
#ifndef BITMAP_PROCES_H
#define BITMAP_PROCES_H

typedef struct 
{
    size_t base;
    size_t bounce;
    size_t heap;
    int pid;
    MemoryManager* memory;
}Process;

// Definición de la estructura del administrador de memoria
typedef struct {
    int MemorySize;
    size_t BonceSize;
    Process* process; // Mapa de bits para el estado de cada bloque
} ProcessManager;

ProcessManager* initializeProcessManager(size_t memory_size, size_t bonce_size);
int NewProcess(ProcessManager* manager, process_t NewProcess);
int freeProcess(ProcessManager* manager, process_t endedProcess);
#endif