#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include "process_bnb.h"
typedef struct List
{
    size_t length;
    size_t size;
    process_bnb *data;
} list;

list *Init_list();
int bnb_Push(list *l, process_bnb v);
int bnb_Is_Process_New(list *l, int pid);
int bnb_RemovePos(list *, size_t);
int bnb_Free(list *);

#endif
