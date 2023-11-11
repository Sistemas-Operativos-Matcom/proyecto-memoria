#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef BITMAP_H
#define BITMAP_H



typedef struct {
    size_t MemorySize;
    size_t BlockSize;
    size_t *memory; // Mapa de bits para el estado de cada bloque
    size_t Stack;
} MemoryManager;

MemoryManager* initializeMemoryManager(size_t memory_size, size_t block_size);
size_t ReserveMemory(MemoryManager* manager, int size) ;
void freeMemory(MemoryManager* manager, int start_block) ;
size_t PushMemory(MemoryManager* manager);
size_t PopMemory(MemoryManager* manager);
// char* printMemory(MemoryManager* manager);
#endif