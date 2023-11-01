#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "Context.c"

typedef struct MyContextList
{
    Context_t* arr; 
    int count; 
    // it has to be big enough
} MyContextList_t; 
MyContextList_t BuildMyList(int m_size, int context_size)
{
    MyContextList_t list;
    // This size may brake...
    list.arr = (Context_t*) malloc(sizeof(Context_t)*(m_size/context_size +1));
    list.count = 0;
    return list;
}
void AddContextToList(MyContextList_t* list, Context_t x)
{
    list->arr[list->count] = x;
    list->count++;
}
void DeleteElementInList(MyContextList_t* list, int pid)
{
    for (int i = 0; i < list->count; i++)
    {
        if(list->arr[i].pid == pid)
        {
            for (int j = i; j+1 < list->count; j++)
            {
                list->arr[j] = list->arr[j+1];
            }
            list->count--;
            return;
        }
    }
}
int ElementExistsInList(MyContextList_t* list, int pid)
{
    for (int i = 0; i < list->count; i++)
    {
        if(list->arr[i].pid == pid)
        {
            return 1;
        }
    }
    return -1;
}
// int main()
// {
//     program_t prog = {
//         "hola",
//         4
//     };
//     process_t proc = {
//         1,
//         &prog
//     };
//     process_t* ptr = &proc;
//     Context_t* con1 = BuildContext(200,0,ptr);
//     ptr->pid = 5;
//     Context_t* con2 = BuildContext(200,0,ptr);

//     context_push(con1, 10);
//     context_push(con1, 21);
//     context_push(con1, 32);

//     int t = context_malloc(con1, 100);
//     int y =context_malloc(con1, 10);
//     context_free(con1,y,10);
//     printf("hello");
//     int u =context_malloc(con1, 2);


//     MyContextList_t list = BuildMyList(1000,200);
//     AddContextToList(&list, *con1);
//     AddContextToList(&list, *con2);
//     AddContextToList(&list, *con1);

//     // It seems like i am deleting contexts even if they are equal.
//     printf("hello");
//     DeleteElementInList(&list,con1->pid);
//     printf("hello");
// }