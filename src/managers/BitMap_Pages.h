#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct 
{
    
}Page;

// Definición de la estructura del administrador de memoria
typedef struct {
    size_t BoundSize;
    Page* pages; // Mapa de bits para el estado de cada bloque
} PagesManager;