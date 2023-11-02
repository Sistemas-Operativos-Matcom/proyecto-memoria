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

void insert(FreeList* freeList, int size);

void print(FreeList* freeList);

struct Node* search(FreeList* freeList, int size);

void delete(FreeList* freeList, struct Node* node);