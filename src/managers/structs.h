#ifndef Structs_h
#define Structs_h

#include "../memory.h"

typedef struct Piece 
{
    int base;
    int bytes[512];
    int process;
} Piece_t;

typedef struct Page
{
    int base;
    int bytes[32];
    int inUse
} Page_t;

typedef struct Proc
{
    int pid;
    Page_t *code;
    int lpcode;
    Page_t *heap;
    int lpheap;
    Page_t *stack;
    int lpstack;
}Proc_t;


#endif