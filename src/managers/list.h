#ifndef LIST_H // Directiva de preprocesador para evitar la inclusión múltiple
#define LIST_H

typedef struct sizeList
{
    int len;      // current size of the array
    size_t *data; // pointer to the array data
    size_t size;  // maximum size that can be stored
} sizeList_t;

// Returns the length (number of elements) of the list
int length(sizeList_t *list);

// Initializes the sizeList_t structure and returns a pointer to it
sizeList_t *init();

// resetea la lista
void reset(sizeList_t *l);

// Returns the element at index i in the list
size_t get(sizeList_t *l, int i);

// Sets the value c at index i in the list
int set(sizeList_t *l, int i, size_t c);

// Checks if the index i is within the valid range of the list
int validIndex(sizeList_t *l, int i);

// Increases the size of the list if necessary
void increaseSize(sizeList_t *l);

// Inserts the element c at index i in the list
int insert(sizeList_t *l, int i, size_t c);

// Appends the element c at the end of the list
void push(sizeList_t *l, size_t c);

// Removes and returns the last element from the list
size_t pop(sizeList_t *l);

// Prints all the elements in the list
void printAll(sizeList_t *l);

// Prints the element at index i in the list
void printAt(sizeList_t *l, int i);

// Deletes the element at index i from the list
int deleteAt(sizeList_t *l, int i);

#endif