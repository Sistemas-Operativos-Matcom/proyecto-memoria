#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include "process_pag.h"
typedef struct List
{
    size_t len;
    size_t size;
    process_pag *data;
} pList;

pList *Init_l_pag();
int Push_l_pag(pList *l, process_pag v);
int RemovePos_l_pag(pList *l, size_t p);
int Contains_l_pag(pList *l, int pid);
int Free_l_pag(pList *l);

#endif
