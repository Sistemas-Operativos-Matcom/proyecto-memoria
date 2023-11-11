#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BitMap.h"
#include "../utils.h"
#include "BitMap_Proces.h"

static size_t BOUNCE = 512;
static FILE* file1;



// Función para inicializar el administrador de memoria
ProcessManager* initializeProcessManager(size_t memory_size, size_t bonce_size) {
    ProcessManager* manager = (ProcessManager*)malloc(sizeof(ProcessManager));
    manager->BonceSize = bonce_size;
    manager->MemorySize = memory_size;
    Process* process = (Process*)malloc(sizeof(Process[memory_size/bonce_size]));
    manager->process = process;
    Process* processnull = (Process*)malloc(sizeof(Process));
    processnull->base = 0;
    processnull->bounce = 0;
    processnull->pid = -1;
    for (size_t i = 0; i < memory_size / bonce_size; ++i) {
        manager->process[i] = *processnull; // Inicializar todos los bloques como libres (0)
    }
    return manager;
}

// Función para asignar memoria
int NewProcess(ProcessManager* manager, process_t NewProcess) {
    int new = -1;
    for (size_t i = 0; i < manager->MemorySize/manager->BonceSize; ++i) {
        // printf("\n%i       %i\n",manager->process[i].pid, NewProcess.pid);
        if (manager->process[i].pid == NewProcess.pid) {
            return i;
        }
        if (new == -1 && manager->process[i].base == 0 && manager->process[i].bounce == 0) {
            new = i;
        }
    }

    Process* process = (Process*)malloc(sizeof(Process));
    process->base = new*BOUNCE;
    process->heap = new*BOUNCE + NewProcess.program->size+1;
    process->bounce = BOUNCE;
    process->pid = NewProcess.pid;
    process->memory = initializeMemoryManager(512 - NewProcess.program->size, 1);;
    manager->process[new] = *process;
    printf("Se ha guardado un nuevo proceso con un base: %ld\n", process->base);
    return new;
}

// Función para liberar memoria
int freeProcess(ProcessManager* manager, process_t endedProcess) {
    Process* processnull = (Process*)malloc(sizeof(Process));
    processnull->base = 0;
    processnull->bounce = 0;
    for (size_t i = 0; i < manager->MemorySize / manager->BonceSize; ++i) {
        if (manager->process[i].pid == endedProcess.pid)
        {
            manager->process[i] = *processnull;
            printf("Proceso liberada\n");
            return i;
        }
    }
    printf("No se encontro el proceso");
    exit(1);
}