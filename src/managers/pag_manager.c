#include <stdio.h>
#include "pag_manager.h"
#include "structs.h"

static lf_list free_list;
static size_t page_size = 256;
static tablepagelist tablelist;
static process_t actual_proc;
static FILE *debug_pag;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
  debug_pag = fopen("./Debug/Debug.txt", "w");
  free_list = Init_LF(m_size());
  tablelist = InitTable();
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  tablepage *table = Find_table(actual_proc, &tablelist);
  out->addr = table->heap;
  out->size = size;
  table->heap += size;
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out)
{

  tablepage *table = Find_table(actual_proc, &tablelist);
  table->stack--;
  page *real = Find_Page((size_t)table->stack / page_size, &table->book);
  m_write(real->addr + table->stack % page_size, val);
  out->addr = table->stack;
  out->size = 1;
  return 0;
}

// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  tablepage *table = Find_table(actual_proc, &tablelist);
  page *real = Find_Page((size_t)(table->stack / page_size), &table->book);
  *out = m_read(real->addr + table->stack % page_size);
  table->stack++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  tablepage *table = Find_table(actual_proc, &tablelist);
  size_t realaddr = Search_addr(addr, &table->mask);
  page *real = Find_Page((size_t)(realaddr / page_size), &table->book);
  *out = m_read(real->addr + realaddr % page_size);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val)
{

  tablepage *table = Find_table(actual_proc, &tablelist);
  Add_Mask(addr, table->heap, &table->mask);
  if (!Exist_Page((size_t)(table->heap / page_size), &table->book))
  {
    lf_element newpage = Fill_Space(page_size, &free_list);
    page newv = {(size_t)addr / page_size, ((size_t)addr / page_size) * page_size};
    page newr = {newpage.start, newpage.start + newpage.size - 1};
    Add_DPage(newv, newr, &table->book);
  }
  page *real = Find_Page((size_t)(table->heap / page_size), &table->book);
  m_write(real->addr + (table->heap % page_size), val);
  table->heap++;
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process)
{
  if (!Exist_table(process, &tablelist))
  {
    lf_element codesize = Fill_Space(page_size, &free_list);
    lf_element stackpage = Fill_Space(page_size, &free_list);
    lf_element heappage = Fill_Space(page_size, &free_list);
    set_curr_owner(process.pid);
    m_set_owner(codesize.start, codesize.start + codesize.size - 1);
    m_set_owner(heappage.start, heappage.start + heappage.size - 1);
    m_set_owner(stackpage.start, stackpage.start + stackpage.size - 1);
    AddTable(process, 0, 4 * page_size, &tablelist);
    page coder = {-1, codesize.start};
    page codev = {-1, 0};
    tablepage *n = Find_table(process, &tablelist);
    Add_DPage(codev, coder, &n->book);
    page heapv = {0, 0};
    page heapr = {-1, heappage.start};
    Add_DPage(heapv, heapr, &n->book);
    page stackv = {3, 3 * page_size};
    page stackr = {-1, stackpage.start + stackpage.size};
    Add_DPage(stackv, stackr, &n->book);
  }
  actual_proc = process;
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  tablepage *removed = Find_table(process, &tablelist);
  for (size_t i = 0; i < removed->book.lenght; i++)
  {
    m_unset_owner(removed->book.start[i].real_page.addr, removed->book.start[i].real_page.addr + page_size - 1);
    Free_Space(removed->book.start[i].real_page.addr, removed->book.start[i].real_page.addr + page_size, &free_list);
  }
  RemoveTable(process, &tablelist);
}
