#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include "process.h"
typedef struct List
{
    size_t len;
    size_t size;
    process_bb *data;
} IntList;

IntList *Init_l();
int Push_l(IntList *l, process_bb v);
int RemovePos_l(IntList *l, size_t p);
int Contains_l(IntList *l, int pid);
int Free_l(IntList *l);

#endif
