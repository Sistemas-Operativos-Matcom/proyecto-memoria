#include <stdio.h>
#include <stdlib.h>

typedef struct Free_List
{
    int* data; 
    int m_size;
} Free_list_t;

Free_list_t Build_Free_List(int m_size)
{
    Free_list_t f;
    f.data = (int*) malloc(sizeof(int)*m_size + 1);
    f.m_size = m_size;  
    for (int i = 0; i < m_size; i++)
    {
        f.data[i] = 0;
    }
    return f;
}
int get_last_position(Free_list_t* f)
{
    int max = 0;
    for (int i = 0; i < f->m_size; i++)
    {
        int x = f->data[i];
        max = (x == 1)? i : max;  
    }
    return max;
}
int find_free_space(Free_list_t* f, int size)
{
    // Va iterando por todas las posciones buscando una cantidad 
    // contigua de espacio que sea igual al size, y toma la 
    // primera que encuentre. 
    int count = 0; 
    for (int i = 0; i < f->m_size; i++)
    {
        count = (f->data[i])? 0 : count+1;
        if (count == size)
            return i-count+1;
    }
    return -1;
}
int allocate_space(Free_list_t* f, int size)
{
    // Retorna el valor de la direccion donde se guardaran los 
    // datos.
    int adr = find_free_space(f,size);
    if(adr != -1)
    {
        for (int i = adr; i < adr+size; i++)
        {
            f->data[i] = 1;
        }
    }
    return adr; 
}

int free_space(Free_list_t* f,int adr, int size)
{
    // Retorna el valor de la direccion donde se guardaran los 
    // datos.
    for (int i = adr; i < adr+size; i++)
    {
        if(f->data[i]== 0)
            return -1;
    }
    for (int i = adr; i < adr+size; i++)
    {
        f->data[i] = 0;
    }
    return 1;
}