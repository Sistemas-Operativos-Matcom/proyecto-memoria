#include <stdio.h>
#include "../memory.h"
#include "../utils.h"

typedef struct Map_address
{
    byte addr_v;
    size_t addr_r;
} map_addr;

typedef struct Mask_addr
{
    map_addr *start;
    size_t length;
    size_t size;
} mask;

typedef struct b_and_b
{
    process_t process;
    size_t base;
    size_t bounds;
    size_t heap;
    mask mask_addr;
    size_t stack;
} bandb;

typedef struct List
{
    bandb *list_start;
    size_t length;
    size_t size;
} List;

typedef struct LF_Element
{
    size_t start;
    size_t size;
    struct LF_Element *previous;
    struct LF_Element *next;

} lf_element;

typedef struct LF_List
{
    lf_element first;

} lf_list;

typedef struct Page
{
    size_t page_num;
    size_t addr;

} page;

typedef struct DPage
{
    page virtural_page;
    page real_page;
} dpage;

typedef struct Book
{
    dpage *start;
    size_t lenght;
    size_t size;
} book;

typedef struct Lib
{
    process_t process;
    book book;
    size_t heap;
    size_t stack;
    mask mask;

} tablepage;

typedef struct TablePageList
{
    tablepage *start;
    size_t length;
    size_t size;

} tablepagelist;

typedef struct Segment
{
    size_t base;
    size_t bound;
} segment;

typedef struct Seg_register
{
    process_t process;
    segment code_segment;
    segment heap_segment;
    segment stack_segment;
    size_t stack_pointer;
    size_t heap_pointer;
    mask mask;
} seg_register;

typedef struct Register_List
{
    seg_register *start;
    size_t length;
    size_t size;
} register_list;

// Métodos de la Lista

List Init();

void Push(bandb value, List *list);

void Remove(bandb value, List *list);

bandb *Find(process_t process, List *list);

int Exist(process_t process, List *list);

void FreeList(List *list);

// Métodos de la Linked List

lf_list Init_LF(size_t size);

lf_element Fill_Space(size_t size, lf_list *list);

void Free_Space(size_t address, size_t size, lf_list *list);

// Métodos de la máscara de direcciones

mask Init_Mask();

void Add_Mask(byte dir_v, size_t dir_r, mask *list);

size_t Search_addr(byte dir_v, mask *list);

void Remove_addr(byte dir_v, mask *list);

// Métodos del Libro
book InitBook();

void Add_DPage(page dir_v, page dir_r, book *book);

void Remove_DPage(page pag, book *book);

void Delete_Page(size_t position, book *book);

int Exist_Page(size_t number, book *book);

page *Find_Page(size_t number, book *book);

int Exist_page(page dir_virtual);

// Métodos de la lista de tablepages

tablepagelist InitTable();

void AddTable(process_t process, size_t heap, size_t stack, tablepagelist *list);

void RemoveTable(process_t process, tablepagelist *list);

tablepage *Find_table(process_t process, tablepagelist *list);

int Exist_table(process_t process, tablepagelist *list);

// Métodos de la lista de registros de segmentos

register_list Init_Register_List();

int Exist_Register(process_t process, register_list *list);

void Add_Register(process_t process, segment code, segment heap, segment stack, register_list *list);

seg_register *Search_Register(process_t process, register_list *list);

void Remove_Register(process_t process, register_list *list);