#ifndef STRUCTS_H
#define STRUCTS_H

#define Error 1
#define Ok 0
#define LAST_ADDR 0xffffffffffffffff

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//----------------------------------------------NODO DE LAS LISTAS----------------------------------------------

typedef struct Node
{
    size_t pos;         //Posición del nodo
    size_t size;        //Tamaño del nodo
    struct Node *prev;  //Nodo anterior
    struct Node *next;  //Nodo siguiente

} Node;

//----------------------------------------------------LISTAS----------------------------------------------------

typedef struct List
{
    Node *head;         //Primer elemento
    Node *tail;         //Último elemento
    size_t size;        //Tamaño de la lista
    size_t max_pos;     //Última posición de la lista (límite)

} List;

//----------------------------------------------FRAGMENTO DE MEMORIA--------------------------------------------

typedef struct M_Fragment
{
    int process_pid;    //Proceso al que pertenece este fragmento de memoria
    int base;           
    int bound;
    int stack_pointer;
    int heap_pointer;
    List heap;

} Fragment;

//---------------------------------------------------PAGINACIÓN--------------------------------------------

typedef struct Pagination_Info
{
    int process_id;
    bool is_process_working;
    bool *is_page_valid;
    size_t total_memory_size;
    size_t stack_pointer;
    size_t* virtual_page_number;

} Pagination;

//-----------------------------------------------------MÉTODOS---------------------------------------------------

// Inicializar una lista
void List_Init(List *list, size_t total_mem);
// Vaciar una lista
void List_RS(List *list, size_t total_mem);
// Obtener el primer fragmento de memoria con suficiente espacio disponible (espacio size)
int Get_Mem(List *list, size_t size, size_t *addr);
// Liberar un fragmento de memoria del tamaño especificado
int Free_Mem(List *list, size_t size, size_t addr);
// Inicializar estructura Pagination
void Pag_Init(Pagination* proc, int pid, size_t size);
// Encuentra el primer espacio con n paginas adyacentes vacias
size_t Find_Empty_Pages(const Pagination* proc, size_t n);

#endif