#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include "process_pag.h"
typedef struct List
{
    size_t length;
    size_t size;
    process_pag *data;
} list;

list *Init_list_of_pages();
int pag_Push(list *l, process_pag proc_to_insert);
int pag_RemovePos(list *l, size_t p);
int pag_Contains(list *l, int pid);
int pag_Free(list *l);

#endif
