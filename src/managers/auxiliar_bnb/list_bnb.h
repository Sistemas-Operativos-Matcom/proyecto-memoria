#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include "process_bnb.h"
typedef struct List
{
    size_t len;
    size_t size;
    process_bb *data;
} pList;

pList *Init_l();
int Push_l(pList *l, process_bb v);
int RemovePos_l(pList *l, size_t p);
int Contains_l(pList *l, int pid);
int Free_l(pList *l);

#endif
