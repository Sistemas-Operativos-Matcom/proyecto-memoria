#include <stdio.h>
#include <malloc.h>
#include "structs.h"

// Métodos de la Lista

List Init() // Inicializa la lista de base and bounds
{
    bandb *start = malloc(64 * sizeof(bandb));
    List new = {start, 0, 64};
    return new;
}

void Delete(size_t position, List *list) // Elimina el elemento en la posición dada
{
    for (size_t i = position; i < list->length - 1; i++)
    {
        list->list_start[i] = list->list_start[i + 1];
    }
    list->length--;
}

void Remove(bandb value, List *list) // Elimina un elemento de la lista
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (value.process.pid == list->list_start[i].process.pid)
        {
            Delete(i, list);
        }
    }
}

bandb *Find(process_t process, List *list) // Devuelve una referencia del objeto que se quiere buscar
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->list_start[i].process.pid == process.pid)
            return &list->list_start[i];
    }
    return NULL;
}

void Check_Size(List *list) // Método Auxiliar de la lista para checkear el tamaño de la lista
{
    if (list->length + 1 >= list->size)
        list = realloc(list->list_start, list->size * 2);
}

void FreeList(List *list) // Libera el espacio de la lista en memoria
{
    free(list->list_start);
}

int Exist(process_t process, List *list) // verifica si un elemento existe en la lista
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->list_start[i].process.pid == process.pid)
            return 1;
    }
    return 0;
}

void Push(bandb value, List *list)
{
    Check_Size(list);
    list->list_start[list->length] = value;
    list->length++;
}

// Métodos de la Linked List
lf_list Init_LF(size_t size)
{
    lf_element node = {0, size, NULL, NULL};
    lf_list list = {node};
    return list;
}

void Free_Space(size_t address, size_t size, lf_list *list)
{
    lf_element *node = &list->first;

    if (address < node->start) // si el espacio a liberar esta antes que el primer nodo
    {
        if (node->start == address + size)
        {
            node->start = address;
            node->size += size;
        }
        else
        {
            lf_element *temp = malloc(sizeof(lf_element));
            *temp = *node;
            lf_element new = {address, size, NULL, temp};
            list->first = new;
        }
    }
    else if (address >= node->start + node->size) // si esta despues del primer nodo
    {
        while (node->next != NULL && node->next->start <= address) // mientras el espacio a liberar sea mayor que el siguiente nodo
        {
            node = node->next; // muevete al siguiente nodo
        }

        if (node->next == NULL)
        {
            if (address == node->start + node->size)
            {
                node->size += size;
            }
            else
            {
                lf_element *new = malloc(sizeof(lf_element));
                lf_element element = {address, size, node, NULL};
                *new = element;
                node->next = new;
            }
        }
        else
        {
            if (node->start + node->size == address)
            {
                node->size += size;
            }
            else if (node->next->start == address + size)
            {
                node->next->start = address;
                node->next->size += size;
            }
            else
            {
                lf_element *temp1 = malloc(sizeof(lf_element));
                lf_element *temp2 = malloc(sizeof(lf_element));
                *temp1 = *node;
                *temp2 = *node->next;
                lf_element *new = malloc(sizeof(lf_element));
                lf_element element = {address, size, temp1, temp2};
                *new = element;
                node->next->previous = new;
                node->next = new;
            }
        }
    }
}

lf_element Fill_Space(size_t size, lf_list *list)
{

    lf_element *node = &list->first;

    while (node->size < size)
    {
        node = node->next;
    }

    if (node->size == size)
    {
        if (node->previous == NULL && node->next == NULL)
        {
            list->first = (lf_element){0, 0, NULL, NULL};
        }

        else if (node->previous == NULL)
        {
            node->next->previous = NULL;
        }
        else if (node->next == NULL)
        {
            node->previous->next = NULL;
        }
        else
        {

            node->previous->next = node->next;
            node->next->previous = node->previous;
            return *node->previous;
        }
        return *node;
    }
    else
    {
        node->size -= size;
        return (lf_element){node->start + node->size, size, NULL, NULL};
    }
}

// Métodos de la máscara de direcciones

mask Init_Mask()
{
    map_addr *start = malloc(64 * sizeof(map_addr));
    mask new = {start, 0, 64};
    return new;
}

void Check_Size_Mask(mask *list) // Método Auxiliar de la lista para checkear el tamaño de la lista
{
    if (list->length + 1 >= list->size)
        list = realloc(list->start, list->size * 2);
}

void Add_Mask(byte dir_v, size_t dir_r, mask *list)
{
    Check_Size_Mask(list);
    map_addr new = {dir_v, dir_r};
    list->start[list->length] = new;
    list->length++;
}

size_t Search_addr(byte dir_v, mask *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->start[i].addr_v == dir_v)
        {
            return list->start[i].addr_r;
        }
    }
    return 0;
}

void Delete_dir(size_t position, mask *list)
{
    for (size_t i = position; i < list->length; i++)
    {
        list->start[i] = list->start[i + 1];
    }
    list->length--;
}

void Remove_addr(byte dir_v, mask *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->start[i].addr_v == dir_v)
        {
            Delete_dir(i, list);
        }
    }
}

book InitBook()
{
    dpage *start = malloc(64 * sizeof(dpage));
    book new = {start, 0, 64};
    return new;
}

void Check_Book_Size(book *book)
{
    if (book->lenght + 1 >= book->size)
        book = realloc(book->start, book->size * 2);
}

void Add_DPage(page dir_v, page dir_r, book *book)
{
    Check_Book_Size(book);
    dpage new = {dir_v, dir_r};
    book->start[book->lenght] = new;
    book->lenght++;
}

void Delete_Page(size_t position, book *book)
{
    for (size_t i = position; i < book->lenght; i++)
    {
        book->start[i] = book->start[i + 1];
    }
    book->lenght--;
}

void Remove_DPage(page pag, book *book)
{
    for (size_t i = 0; i < book->lenght; i++)
    {
        if (book->start[i].virtural_page.addr == pag.addr)
        {
            Delete_Page(i, book);
        }
    }
}

int Exist_Page(size_t number, book *book)
{
    for (size_t i = 0; i < book->lenght; i++)
    {
        if (book->start[i].virtural_page.page_num == number)
        {
            return 1;
        }
    }
    return 0;
}

page *Find_Page(size_t number, book *book)
{
    for (size_t i = 0; i < book->lenght; i++)
    {
        if (book->start[i].virtural_page.page_num == number)
        {
            return &book->start[i].real_page;
        }
    }
    return NULL;
}

// Métodos de la lista de TablePages
tablepagelist InitTable()
{
    tablepage *start = malloc(64 * sizeof(tablepage));
    tablepagelist new = {start, 0, 64};
    return new;
}

void Check_size_table(tablepagelist *list)
{
    if (list->length + 1 >= list->size)
        list = realloc(list->start, list->size * 2);
}

void AddTable(process_t process, size_t heap, size_t stack, tablepagelist *list)
{
    Check_size_table(list);
    book newbook = InitBook();
    mask newmask = Init_Mask();
    tablepage new = {process, newbook, heap, stack, newmask};
    list->start[list->length] = new;
    list->length++;
}

void DeleteTable(size_t position, tablepagelist *list)
{
    for (size_t i = position; i < list->length; i++)
    {
        list->start[i] = list->start[i + 1];
    }
    list->length--;
}

void RemoveTable(process_t process, tablepagelist *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (process.pid == list->start[i].process.pid)
        {
            DeleteTable(i, list);
        }
    }
}

tablepage *Find_table(process_t process, tablepagelist *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (process.pid == list->start[i].process.pid)
        {
            return &list->start[i];
        }
    }
    return NULL;
}

int Exist_table(process_t process, tablepagelist *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (process.pid == list->start[i].process.pid)
        {
            return 1;
        }
    }
    return 0;
}

// Métodos de la lista de registros

register_list Init_Register_List()
{
    seg_register *start = malloc(64 * sizeof(seg_register));
    register_list new = {start, 0, 64};
    return new;
}

void Check_size_register(register_list *list)
{
    if (list->length + 1 >= list->size)
        list = realloc(list->start, list->size * 2);
}

void Add_Register(process_t process, segment code, segment heap, segment stack, register_list *list)
{
    Check_size_register(list);
    mask newmask = Init_Mask();
    seg_register new = {process, code, heap, stack, stack.base + stack.bound - 1, heap.base, newmask};
    list->start[list->length] = new;
    list->length++;
}

seg_register *Search_Register(process_t process, register_list *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->start[i].process.pid == process.pid)
        {
            return &list->start[i];
        }
    }
    return NULL;
}

int Exist_Register(process_t process, register_list *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->start[i].process.pid == process.pid)
            return 1;
    }
    return 0;
}

void Delete_Register(size_t position, register_list *list) // Elimina el elemento en la posición dada
{
    for (size_t i = position; i < list->length - 1; i++)
    {
        list->start[i] = list->start[i + 1];
    }
    list->length--;
}

void Remove_Register(process_t process, register_list *list)
{
    for (size_t i = 0; i < list->length; i++)
    {
        if (list->start[i].process.pid == process.pid)
        {
            Delete_Register(i, list);
        }
    }
}
