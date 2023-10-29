#include <stdio.h>
#include "../memory.h"
#include "../utils.h"

typedef struct b_and_b
{
    process_t process;
    byte base;
    byte bounds;
    byte heap;
    byte stack;
} bandb;

typedef struct List
{
    bandb *list_start;
    int length;
    int size;
} List;

typedef struct LF_Element
{
    byte start;
    byte size;
    struct LF_Element *previous;
    struct LF_Element *next;
} LFelement;

typedef struct LF_List
{
    LFelement *first;

} LFList;

// Métodos de la Lista

List Init();

void Push(bandb value, List *list);

void Remove(bandb value, List *list);

bandb *Find(process_t process, List *list);

int Exist(process_t process, List *list);

void FreeList(List *list);

// Métodos de la Linked List

LFList Init_LF(byte size);

LFelement *Fill_Space(byte size, LFList *list);

void Free_Space(byte address, byte size, LFList *list);
