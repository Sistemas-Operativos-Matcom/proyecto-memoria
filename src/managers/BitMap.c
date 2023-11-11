#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BitMap.h"
#include "../memory.h"
#include "../utils.h"
#include "BitMap_Proces_Pages.h"


// Función para inicializar el administrador de memoria
MemoryManager* initializeMemoryManager(size_t memory_size, size_t block_size) {
    MemoryManager* manager = (MemoryManager*)malloc(sizeof(MemoryManager));
    manager->MemorySize = memory_size;
    manager->BlockSize = block_size;
    size_t* memory = (size_t*)malloc(sizeof(size_t[memory_size/block_size]));
    manager->memory = memory;
    manager->Stack = memory_size/block_size -1;
    for (size_t i = 0; i < manager->MemorySize / manager->BlockSize; ++i) {
        manager->memory[i] = 0; // Inicializar todos los bloques como libres (0)
    }
    return manager;
}

// Función para asignar memoria
size_t ReserveMemory(MemoryManager* manager, int size) {
    int consecutive_blocks = 0;
    size_t addres = 1;
    for (size_t i = 0; i < manager->MemorySize; ++i) {
        if (manager->memory[i] == 0) {
            consecutive_blocks++;
            if (consecutive_blocks == size) {
                // Marcar los bloques como asignados (1)
                for (size_t j = i - size + 1; j <= i; ++j) {
                    if (j == i - size + 1){
                        manager->memory[j] = size;
                        addres = j;
                    }
                    else {
                        manager->memory[j] = 1;
                    }
                }
                printf("\nMemoria asignada desde el bloque %ld al bloque %ld\n", i - size + 1, i);
                return addres;
            }
        } else {
            consecutive_blocks = 0;
        }
    }
    printf("No hay suficiente memoria disponible.\n");
    exit(1);
}

size_t PushMemory(MemoryManager* manager)
{
    printf("%li", manager->memory[manager->Stack]);
    manager->memory[manager->Stack] = 1;
    printf("%li", manager->memory[manager->Stack]);

    manager->Stack -= 1;
    return manager->Stack;
}

size_t PopMemory(MemoryManager* manager)
{
    manager->Stack += 1;
    manager->memory[manager->Stack] = 0;
    return manager->Stack;
}

// Función para liberar memoria
void freeMemory(MemoryManager* manager, int start_block) {
    int final = start_block + manager->memory[start_block];
    for (int i = start_block; i < final; ++i) {
        printf("\n%li",manager->memory[i]);
        manager->memory[i] = 0; // Marcar bloques como libres
    }
    printf("Memoria liberada\n");
}


// char* printMemory(MemoryManager* manager)
// {
//     char* string = "";
//     for( int i = 0; i<1024/32; i++)
//     {
//         string += ("%d", (uint8_t)(manager->memory[i]));
//     }
//     return string;
// }

// Ejemplo de uso
// int main() {
//     MemoryManager* manager = initializeMemoryManager(1024, 32);

//     ReserveMemory(manager, 128); // Asignar 64 bytes de memoria
//     ReserveMemory(manager, 128); // Asignar 128 bytes de memoria
//     freeMemory(manager, 0); // Liberar memoria desde el bloque 0 al bloque 1
//     ReserveMemory(manager, 64); // Asignar 32 bytes de memoria
//     return 0;
// }
