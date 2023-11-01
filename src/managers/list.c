#include <stdio.h>
#include <stdlib.h>
#include "linkedList.h"

typedef struct
{
    int len; // tamaño actual del array
    char *data;
    int size; // tamaño maximo que se puede guardar
} charList;

// Devuelve la cantidad de elementos de la lista
int length(charList *list)
{
    return list->len;
}

// Inicializa la estructura
charList *init()
{
    charList *l = (charList *)malloc(sizeof(charList));
    l->len = 0;
    l->size = 10;
    l->data = (char *)malloc(l->size * sizeof(char));
    return l;
}

// Devuelve el caracter de una posición
char get(charList *l, int i)
{
    if (i >= 0 && i < l->len)
        return l->data[i];
    else
        return -1;
}

// Setea un valor en una posición
int set(charList *l, int i, char c)
{
    if (i >= 0 && i < l->len)
    {
        l->data[i] = c;
        return 0;
    }
    else
        return -1;
}

// Verifica que el índice se encuentre dentro del tamaño de la lista
int validIndex(charList *l, int i)
{
    if (i < 0 || i > l->len) // check i is valid
        return -1;
    return 0;
}

// Aumenta el tamaño de la lista si es necesario
void increaseSize(charList *l)
{
    if (l->size == l->len)
    {
        l->size = l->size * 2;
        l->data = (char *)realloc(l->data, l->size * sizeof(char));
    }
}

// Inserta un elemento en una posición definida
int insert(charList *l, int i, char c)
{
    // index was invalid
    if (validIndex(l, i) == -1)
        return -1;

    increaseSize(l);

    for (int j = l->len; j > i; j--)
        l->data[j] = l->data[j - 1];

    l->data[i] = c;
    l->len++;

    return 0;
}

// Inserta un elemento al final de la lista
void push(charList *l, char c)
{
    increaseSize(l);
    l->data[length(l)] = c;
    l->len++;
}

// Función para imprimir todos los elementos de la lista
void printAll(charList *l)
{
    printf("La lista tiene %d elementos:\n", l->len);
    for (int i = 0; i < l->len; i++)
    {
        printf("%c ", l->data[i]);
    }
    printf("\n");
}

// Función para imprimir el elemento de una posición específica de la lista
void printAt(charList *l, int i)
{
    if (i >= 0 && i < l->len)
    {
        printf("El elemento en la posición %d es: %c\n", i, l->data[i]);
    }
    else
    {
        printf("Posición inválida\n");
    }
}

// Función para eliminar el elemento de una posición específica de la lista
int deleteAt(charList *l, int i)
{
    if (i < 0 || i >= l->len)
    { // Verificar si la posición es válida
        return -1;
    }

    // Desplazar los elementos a la izquierda para sobrescribir el elemento en la posición i
    for (int j = i; j < l->len - 1; j++)
    {
        l->data[j] = l->data[j + 1];
    }

    l->len--; // Reducir el tamaño de la lista

    // Si el tamaño de la lista es menor que la mitad del tamaño del array, reducir el tamaño del array a la mitad
    if (l->len < l->size / 2)
    {
        l->size /= 2;
        l->data = (char *)realloc(l->data, l->size * sizeof(char));
    }

    return 0;
}

int main()
{

    // struct LinkedList list;
    // append(&list, 13);
    // append(&list, 15);
    // printList(&list);
    // deleteLastNode(&list);
    // printList(&list);

    // charList* l = init();
    // printAll(l);
    // printAt(l, 94);
    // deleteAt(l, 5);
    // printAt(l, 5);

    return 0;
}
