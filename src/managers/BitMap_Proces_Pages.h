#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BitMap.h"
#include "BitMap_Proces.h"
#include "list.h"

typedef struct 
{
    ListPage* Heap;
    ListPage* Code;
    ListPage* Stack;
    int pid;
}Process_Page;


// Definición de la estructura del administrador de memoria
typedef struct {
    int MemorySize;
    size_t BonceSize;
    Process_Page* process; // Mapa de bits para el estado de cada bloque
    int* table;
} ProcessPageManager;

ProcessPageManager* initializeProcessPageManager(size_t memory_size, size_t bonce_size);
int NewProcessPage(ProcessPageManager* manager, process_t NewProcess);
void freeProcessPage(ProcessPageManager* manager, process_t endedProcess);
size_t ReservePageMemory(Process_Page process, size_t size,int* table, size_t CantidadPage, int heap_stack);
size_t realaddrs(ListPage* heap, addr_t addr);
void freeMemoryPage(ListPage* Heap, int start_block);

