#include <stdio.h>
#include "../memory.h"
#include "../utils.h"

typedef struct Map_address
{
    byte addr_v;
    size_t addr_r;
} map_addr;

typedef struct Mask_addr
{
    map_addr *start;
    size_t length;
    size_t size;
} mask;

typedef struct b_and_b
{
    process_t process;
    size_t base;
    size_t bounds;
    size_t heap;
    mask mask_addr;
    size_t stack;
} bandb;

typedef struct List
{
    bandb *list_start;
    size_t length;
    size_t size;
} List;

typedef struct LF_Element
{
    size_t start;
    size_t size;
    struct LF_Element *previous;
    struct LF_Element *next;
} LFelement;

typedef struct LF_List
{
    LFelement first;

} LFList;

// Métodos de la Lista

List Init();

void Push(bandb value, List *list);

void Remove(bandb value, List *list);

bandb *Find(process_t process, List *list);

int Exist(process_t process, List *list);

void FreeList(List *list);

// Métodos de la Linked List

LFList Init_LF(size_t size);

LFelement Fill_Space(size_t size, LFList *list);

void Free_Space(size_t address, size_t size, LFList *list);

//

mask Init_Mask();

void Add_Mask(byte dir_v, size_t dir_r, mask *list);

size_t Search_addr(byte dir_v, mask *list);

void Remove_addr(byte dir_v, mask *list);