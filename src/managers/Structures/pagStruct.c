#include <stdio.h>
#include <stdlib.h>
#include "pagStruct.h"

// Inicializa la estructura pagStruct
pagStruct* init_pagProcs_array(int size)
{
    // Asigna memoria para el array de pagStruct
    pagStruct* procs_array = (pagStruct*)malloc(size * sizeof(pagStruct));
    return procs_array;
}
