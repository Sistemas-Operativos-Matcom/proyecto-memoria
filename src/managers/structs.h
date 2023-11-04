#ifndef Structs_h
#define Structs_h

#include "../memory.h"

typedef struct Piece 
{
    int base;
    int bytes[1024];
    int process;
} Piece_t;

// typedef struct List
// {
//     Piece_t *pieces;
// }List_t;

#endif