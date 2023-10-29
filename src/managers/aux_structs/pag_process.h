#ifndef PAG_PROCESS_H
#define PAG_PROCESS_H

#include <stdlib.h>
#include <stdbool.h>

// Esta estructura almacena los datos referentes a un proceso y su 
// tabla de paginacion. Tambien guarda otros datos como la validez
// de las paginas, el tamaño maximo, si el proceso esta activo...
typedef struct pag_process
{
    int pid;
    bool is_active;
    size_t total_mem;
    size_t stack_point;
    size_t* vpn;
    bool* valid;
} pag_process_t;

// Inicializa un pag_process con datos nuevos
void pag_process_init(pag_process_t* proc, int pid, size_t size);
// Encuentra el primer espacio con size paginas contiguas vacias
size_t find_space(const pag_process_t* proc, size_t size);
// Calcula el logaritmo en base 2 para enteros
// notar que no funciona para numeros negativos ademas que es
// unsigned en tipo de retorno y en parametros
size_t _log2(size_t n);

#endif