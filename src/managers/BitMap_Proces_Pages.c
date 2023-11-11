#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BitMap.h"
#include "../utils.h"
#include "BitMap_Proces.h"
#include "BitMap_Proces_Pages.h"

static size_t BOUND = 512;



// Función para inicializar el administrador de memoria
ProcessPageManager* initializeProcessPageManager(size_t memory_size, size_t size) {
    ProcessPageManager* manager = (ProcessPageManager*)malloc(sizeof(ProcessPageManager));
    manager->BonceSize = size;
    manager->MemorySize = memory_size;
    int* table = (int*)malloc(sizeof(memory_size/size));
    Process_Page* process = (Process_Page*)malloc(sizeof(Process_Page[memory_size/size]));
    manager->process = process;
    for (size_t i = 0; i < memory_size/size; ++i) {
        Process_Page* processnull = (Process_Page*)malloc(sizeof(Process_Page));
        processnull->pid = -1;
        manager->process[i] = *processnull; // Inicializar todos los bloques como libres (0)
        table[i] = -2;
    }
    manager->table = table;

    return manager;
}

// Función para asignar memoria
int NewProcessPage(ProcessPageManager* manager, process_t NewProcess) {
    int newTable = -1;
    int newProc = -1;
    for (size_t i = 0; i < manager->MemorySize/manager->BonceSize; ++i) {
        // printf("\n%i       %i\n",manager->process[i].pid, NewProcess.pid);
        if (manager->process[i].pid == NewProcess.pid) {
            return i;
        }
        if (newTable == -1 && manager->table[i] == -2) {
            newTable = i;
        }
        if (newProc == -1 && manager->process[i].pid == -1) {
            newProc = i;
        }
    }

   

    Process_Page* process = (Process_Page*)malloc(sizeof(Process_Page));
    ListPage* code = createList(newTable*(manager->MemorySize/manager->BonceSize));
    ListPage* Heap = createList(99999);
    ListPage* Stack = createList(99999);
    process->Heap = Heap;
    process->Stack = Stack;
    process->pid = NewProcess.pid;
    process->Code = code;
    manager->table[newTable] = NewProcess.pid;
    manager->process[newProc] = *process;
    m_set_owner(newTable*(manager->BonceSize), ((newTable+1)*(manager->BonceSize))-1);
    
    
    // process->memory = initializeMemoryManager(512 - NewProcess.program->size, 1);;
    // manager->process[new] = *process;
    printf("\nSe ha guardado un nuevo proceso en la pagina: %d, que empieza en %ld\n", newTable, newTable*(manager->BonceSize));
    return newProc;
}

// Función para liberar memoria
void freeProcessPage(ProcessPageManager* manager, process_t endedProcess) {
    Process_Page* processnull = (Process_Page*)malloc(sizeof(Process));
    processnull->pid = -1;

    for (size_t i = 0; i < manager->MemorySize / manager->BonceSize; ++i)
    {
        if(manager->table[i] == endedProcess.pid)
        {
            manager->table[i] = -2;
            printf("\nSe ha borrado el proceso: %i de la pagina: %li", endedProcess.pid, i);
            m_unset_owner(i*(manager->BonceSize), ((i+1)*(manager->BonceSize))-1);
        }
    }

    for (size_t i = 0; i < manager->MemorySize / manager->BonceSize; ++i) {
        if (manager->process[i].pid == endedProcess.pid)
        {
            manager->process[i] = *processnull;
            printf("\nProceso liberado");
        }
    }
    
}

size_t ReservePageMemory(Process_Page process, size_t size_valor, int* table, size_t CantidadPage, int heap_stack)
{
    printf("\nRESERVANDO MEMORIA\n");
    ListPage* MemoryPage;
    if (heap_stack) {MemoryPage = process.Heap;} 
    else {MemoryPage = process.Stack;}
    int o=0;
    if (MemoryPage->start->Node == 99999)
    {
        for (size_t i = 0; i < CantidadPage; i++)
        {
            if (table[i] == -2)
            {
                m_set_owner(i*256, ((i+1)*256)-1);
                printf("Se reservo la memoria desde %li hasta %li", i*256, ((i+1)*256)-1);
                table[i] = process.pid;
                o = i*256;
                MemoryPage->start->virtualMemory = initializeMemoryManager(256, 1);
                break;
            }
        }   
        MemoryPage->start->Node = o;
    }
    Page* page = MemoryPage->start;
    int NumPage = 1;
    size_t consecutive_blocks = 0;
    size_t addres = 9999999;
    int addresPage = 0;
    while (1)
    {
                // printf("\n%ldAAAAAAAAA\n", i);
        for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {

            if (page->virtualMemory->memory[i] == 0) {
                if (heap_stack)
                {

                    consecutive_blocks++;
                        // printf("%li", size_valor);
                    if (consecutive_blocks == size_valor) {
                        // return 0;
                        // Marcar los bloques como asignados (1)
                        for (size_t j = i - size_valor + 1; j <= i; ++j) {
                            if (j == i - size_valor + 1){
                                if (addres==9999999)
                                {
                                    page->virtualMemory->memory[j] = size_valor;
                                    addres = j;
                                    addresPage = NumPage;
                                }
                                else
                                {
                                    page->virtualMemory->memory[j] = 1;
                                }
                            }
                            else {
                                page->virtualMemory->memory[j] = 1;
                            }
                        }
                        printf("\nMemoria asignada desde el bloque %ld al bloque %ld con adres: %ld \n", i - size_valor + 1, i, addres);
                        goto fin;
                    }
                }
                else
                {
                    page->virtualMemory->memory[i] = 1;
                    page->virtualMemory->Stack = i;
                    return i;
                }
            } else {
                consecutive_blocks = 0;
            }
        }
        for (size_t j = page->virtualMemory->MemorySize - consecutive_blocks; j <= page->virtualMemory->MemorySize; ++j) {
            if (j == page->virtualMemory->MemorySize - consecutive_blocks && addres == 9999999 && consecutive_blocks != 0){
                // printf("\nConsecutive %ld\n", consecutive_blocks);
                page->virtualMemory->memory[j] = size_valor;
                addres = j;
                addresPage = NumPage;
            }
            else {
                page->virtualMemory->memory[j] = 1;
            }
        }
        // for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {
        //     printf("%li  ",page->virtualMemory->memory[i]);
        // }
        insertPage(MemoryPage, table, CantidadPage, process.pid);
        // printf("\n%ld\n", MemoryPage->size);

        page = page->next;
        NumPage ++;
        size_valor = size_valor - consecutive_blocks;
        // printf("\nNext Page %ld", consecutive_blocks);
        consecutive_blocks = 0;
        // printf("\nNext Page %ld", size_valor);
        // return 0;
        // printf("\n");
        // for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {
        //     printf("%li  ",page->virtualMemory->memory[i]);
        // }
    }
    fin:
    //     printf("\n");
    // for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {
    //     printf("%li  ",page->virtualMemory->memory[i]);
    // }
    return addres+ (256*(addresPage-1));
}


void freeMemoryPage(ListPage* Heap, int start_block) {
    // printf("\nLIBERANDO MEMORIA\n");
    Page* page = Heap->start;
    int z = 0;
    while (z < start_block/256)
    {
        page = page->next;
        z++;
    }

    for (size_t i = 0; i < Heap->start->virtualMemory->MemorySize; i++)
    {
        if (i == (size_t)start_block%256)
        {
            z = i;
        }
    }
    size_t size_valor = page->virtualMemory->memory[z];
    int final = start_block + size_valor;
    // printf("\nValor %ld\n", size_valor);
    int NumPage = 1;
    size_t consecutive_blocks = 0;
    // if(start_block!=140 && start_block!=80 && start_block!=50 && size_valor!=50 && size_valor != 1000)
    // {
    //     printf("\n%ldMMMMMMMMMMMMMMMMMMMMMMMMM\n", size_valor);
    //     exit(1);
    // }
    while (1)
    {
    //             // printf("\n%ldAAAAAAAAA\n", i);
        for (size_t i = z; i < page->virtualMemory->MemorySize; ++i) {
                    // return 0;
                    // Marcar los bloques como asignados (1)
            page->virtualMemory->memory[i] = 0;
            size_valor-=1;
            if (size_valor == 0)
            {
        // printf("\n");
        // for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {
        //     printf("%li  ",page->virtualMemory->memory[i]);
        // }
                return;
            }
        }
        // printf("\n");
        // for (size_t i = 0; i < page->virtualMemory->MemorySize; ++i) {
        //     printf("%li  ",page->virtualMemory->memory[i]);
        // }  
            // printf("\nSize_valor %ld\n", size_valor);
        z = 0;
        // printf("\n%ld\n", MemoryPage->size);

        page = page->next;
        // printf("\nNext Page %ld", consecutive_blocks);
        // printf("\nNext Page %ld", size_valor);
    }
    printf("Memoria liberada\n");
}

size_t realaddrs(ListPage* heap, addr_t addr)
{
    // printf("\nNode %i address: %d\n", heap->start->Node, addr);
    size_t pagez = addr/256;
    if (addr%256 == 0)
    {
        pagez += 1;
    }
    Page* page = heap->start;
    for (size_t i = 0; i < pagez; i++)
    {
        page = page->next;
    }
    return page->Node + addr%256;
}