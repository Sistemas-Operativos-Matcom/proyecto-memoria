#ifndef STACK_H
#define STACK_H

typedef struct 
{
    int* stack_elements;    //Array con los elementos del stack
    int size;   // Tamaño máximo del stack
    int len;    // Tamaño actual del stack
    int tail;   // Último elemento del stack
    int head;   // Primer elemento del stack
} stack;

stack* initialize(int size);
int is_empty(stack* s);
int size(stack* s);
void push(stack* s, int element);
int pop(stack* s);
int get_tail(stack* s);


#endif