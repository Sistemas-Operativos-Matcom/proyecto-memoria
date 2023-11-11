#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include "../memory.h"
ListPage* createList(int start)
{
    Page* node = (Page*)malloc(sizeof(Page));
    node->next = node;
    node->Node = start;
    ListPage* list = (ListPage*)malloc(sizeof(ListPage));
    list->start = node;
    list->head = node;
    list->size = 1;
    return list;
}

void insertPage(ListPage* list, int*table, int CantidadPage, int pid)
{
    Page* node = (Page*)malloc(sizeof(Page));
    node->next = node;
    int j = 0;
    for (int i = 0; i < CantidadPage; i++)
    {
        if (table[i] == -1)
        {
            j= i;
        }
    }
    m_set_owner(j*256, ((j+1)*256)-1);
    table[j] = pid;
    node->virtualMemory = initializeMemoryManager(256, 1);
    node->Node = j*256;
    list->head->next = node;
    list->head = node;
    list->size += 1;
    return;
}
void deleteLastPage(ListPage* list)
{
    Page* page = list->start;
    while (1)
    {
        if( (*page->next).next == page->next)
        {
            break;
        }
        page = page->next;
    }
    list->head = page;
    page->next = page;
    list->size -= 1;
}
// int main()
// {
//     ListPage* list = createList(25);
//     insertPage(list, 50);
//     insertPage(list, 75);
//     deleteLastPage(list);
//     insertPage(list, 75);
//     Page* page = list->start;
//     while (1)
//     {
//         if( page->next == page)
//         {
//             printf("\n%i 0 ",page->Node);
//             break;
//         }
//         printf("\n%i",page->Node);
//         page = page->next;
//     }
//     printf("\nHola");
//     return 0;
// }