#include "BitMap.h"
#include <stdlib.h>

typedef struct Page
{
    int Node;
    MemoryManager* virtualMemory;
    struct Page* next;
}Page;

typedef struct
{
    Page* start;
    Page* head;
    size_t size;
}ListPage;


ListPage* createList(int start);

void insertPage(ListPage* list, int*table, int CantidadPage, int pid);
void deleteLastPage(ListPage* list);
