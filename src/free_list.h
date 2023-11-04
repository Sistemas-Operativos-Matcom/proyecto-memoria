#include <stdio.h>
#include <stdlib.h>

struct Node {
    int value;
    int size;
    struct Node* next;
};

typedef struct {
    struct Node* head;
} FreeList;

void insert(FreeList* freeList, int value, int size);

void print(FreeList* freeList);

struct Node* search(FreeList* freeList, int size);
struct Node* searchL(FreeList* freeList);
struct Node* grownIf(FreeList* freeList, int size);
struct Node* grownThis(FreeList* freeList, int value);

void delete(FreeList* freeList, struct Node* node);
void deleteLast(FreeList* freeList);