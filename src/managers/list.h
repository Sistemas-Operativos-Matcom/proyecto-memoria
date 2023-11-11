#ifndef LIST_H // Directiva de preprocesador para evitar la inclusión múltiple
#define LIST_H

#include <stddef.h>

typedef struct sizeList
{
    size_t len;   // current size of the array
    size_t *data; // posize_ter to the array data
    size_t size;  // maximum size that can be stored
} sizeList_t;



// Initializes the sizeList_t structure and returns a posize_ter to it
sizeList_t *init();

// resetea la lista
void reset(sizeList_t *l);

// Returns the element at index i in the list
size_t get(sizeList_t *l, size_t i);

// Sets the value c at index i in the list
void set(sizeList_t *l, size_t i, size_t c);



// Increases the size of the list if necessary
void increaseSize(sizeList_t *l);


// Appends the element c at the end of the list
void push(sizeList_t *l, size_t c);

// Removes and returns the last element from the list
size_t pop(sizeList_t *l);



// Deletes the element at index i from the list
void deleteAt(sizeList_t *l, size_t i);

#endif