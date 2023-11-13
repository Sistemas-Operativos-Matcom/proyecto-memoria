#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

// Inicializa la pila
stack* initialize(int size) 
{
    stack* s = (stack*)malloc(sizeof(stack));
    s->stack_elements = (int*)malloc(size * sizeof(int));
    s->size = size;
    s->len = 0;
    s->head = 0;
    s->tail = -1;
    return s;
}

// Verifica si la pila está vacía
int is_empty(stack* s) 
{
    return s->len == 0;
}

// Devuelve el tamaño de la pila
int size(stack* s) 
{
    return s->len;
}

// Agrega un elemento a la pila
void push(stack* s, int element) 
{
    if (s->len == s->size) {
        printf("La pila está llena.\n");
        return;
    }
    s->len++;
    s->tail++;
    s->stack_elements[s->tail] = element;
}

// Elimina un elemento de la pila
int pop(stack* s) 
{
    if (is_empty(s)) {
        printf("La pila está vacía.\n");
        return -1;
    }
    int element = s->stack_elements[s->tail];
    s->len--;
    s->tail--;
    return element;
}

// Devuelve el último elemento insertado al stack
int get_tail(stack* s)
{
    return s->stack_elements[s->tail];
}