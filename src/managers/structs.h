#ifndef STRUCTS_H
#define STRUCTS_H

#define Error 1
#define Ok 0

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//----------------------------------------------NODO DE LAS LISTAS----------------------------------------------

typedef struct Node
{
    size_t pos;
    size_t size;
    struct Node *prev;
    struct Node *next;
} Node;

//----------------------------------------------------LISTAS----------------------------------------------------

typedef struct List
{
    Node *head;
    Node *tail;

    size_t size;
    size_t max_pos;

} List;

//----------------------------------------------FRAGMENTO DE MEMORIA--------------------------------------------

typedef struct M_Fragment
{
    int process_pid;
    int base;
    int bound;
    int stack_pointer;
    int heap_pointer;
    List heap;
} Fragment;

//-----------------------------------------------------MÉTODOS---------------------------------------------------

// Inicializa una lista
void List_Init(List *list, size_t total_mem);
// Vacia una lista
void List_RS(List *list, size_t total_mem);
// Obtener el primer fragmento de memoria con suficiente espacio disponible (espacio size)
int Get_Mem(List *list, size_t size, size_t *addr);
// Se libera (de ser posible) un trozo de memoria de tamaño size
// que se encuentre en la posicion addr
int Free_Mem(List *list, size_t size, size_t addr);

#endif