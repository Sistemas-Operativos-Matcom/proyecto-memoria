#include "pag_manager.h"
#include "../memory.h"
#include "stdio.h"

//#define __MAX_COUNT_PAGES__ 200
#define __PROCESS_MAX__ 30
#define __PAGE_LENGTH__ 32
#define __COUNT_INIT_PAGES__ 4


typedef struct node //cada nodo representa un page frame
{
  addr_t page_num;
  int free;
  addr_t offset;
  struct node *next;
}Node;

Node *newNode(addr_t page_num)
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->page_num = page_num;
  node->free = 0;
  node->offset = 0;
  node->next = NULL;
  return node;
}

typedef struct list //estructura de la lista de nodos (pages frames)
{
  Node *root;
  int count;
  Node *last;
}List;

List *newList()
{
  List *list = (List *)malloc(sizeof(List));
  list->count = 0;
  list->last = NULL;
  list->root = NULL;
  return list;
}

Node *getPageFrame(Node *node)
{
  if(node->free == 0) 
  {
    node->free = 1;
    return node;
  }
  addr_t addr = -1;
  return newNode(addr);
}

void Add(List *list, Node *node)
{
  if(list->count != 0) list->last->next = node;
  
  else
  {
    list->root = node;
  }

  list->last = node;
  list->count ++;
}

void InsertRec(List *list, Node *node, int index, Node *actual_n, int actual_i)
{
  if(actual_i == index - 1)
  {
    node->next = actual_n->next;
    actual_n->next = node;
    return;
  }
  InsertRec(list, node, index, actual_n->next, actual_i++);
}

void Insert(List *list, Node *node, int index)
{
  if(index < 0 || index > list->count) return;
  if(index == 0)
  {
    node->next = list->root;
    list->root = node;
  }
  else if (index == list->count) Add(list, node);

  else InsertRec(list, node, index, list->root, 0);

  list->count ++;
}

void DeleteRec(List *list, Node *node, Node *actual_n)
{
  if(actual_n->next == node)
  {
    actual_n->next = node->next;
    if(node == list->last) list->last = actual_n;
    list->count --;
    return;
  }
  if(actual_n == list->last) return; //el nodo a eliminar no esta en la lista

  DeleteRec(list, node, actual_n->next);
}

void Delete(List *list, Node *node)
{
  if(list->count == 0) return;
  if(list->count == 1)
  {
    if(node != list->root) return;
    list->root = NULL;
    list->last = NULL;
    list->count = 0;
    return;
  }
  
  if(node == list->root)
  {
    node->next = list->root->next;
    list->root = node;
    list->count --;
    return;
  } 
  DeleteRec(list, node, list->root);  
}

void findAddrRec(addr_t addr,  Node *node, Node *actual_n)
{
  if(actual_n == NULL) return;
  if(actual_n->page_num <= addr && addr < actual_n->page_num + __PAGE_LENGTH__)
  {
    node = actual_n;
    return;
  }
  findAddrRec(addr, node, actual_n->next);
}

void newDirSpaceRec(Node *actual_page_frame, int actual_count /*cantidad de paginas agregadas*/, int total_pages, List *dir_space)
{
  if(actual_count == total_pages) return;

  if(actual_page_frame->free == 1) newDirSpaceRec(actual_page_frame->next, actual_count, total_pages, dir_space);
  if(actual_page_frame->free == 0)
  {
    actual_page_frame->free = 1;
    Node *node = newNode(actual_page_frame->page_num);
    node->free = 1;
    Add(dir_space, node);
    newDirSpaceRec(actual_page_frame->next, actual_count ++, total_pages, dir_space);
  }
}

void m_set_owner_rec(Node *node)
{
  if(node == NULL) return;
  m_set_owner(node->page_num, node->page_num + __PAGE_LENGTH__);
  m_set_owner_rec(node->next);
}

void m_unset_owner_rec(Node *node)
{
  if(node == NULL) return;
  m_unset_owner(node->page_num, node->page_num + __PAGE_LENGTH__);
  m_unset_owner_rec(node->next);
}

void freeRec(addr_t *free_list, Node *node)
{
  if(node == NULL) return;
  for (size_t i = node->page_num; i < node->page_num + __PAGE_LENGTH__; i++)
  {
    free_list[i] = 0;
  }
  freeRec(free_list, node->next);  
}

int suficentPageRec(Node *actual_n, size_t size, Node *node)
{
  if(actual_n == NULL) 
  {
    node = actual_n;
    return 0;
  }
  if(actual_n->offset + size < __PAGE_LENGTH__)
  {
    node = actual_n;
    return 1;
  } 
  if(actual_n->offset + size >= __PAGE_LENGTH__ && actual_n->next != NULL)
  {
    node = actual_n;
    return 1;
  }
  return suficentPageRec(actual_n->next, size, node);
}

void actualizeOffset(Node *node, Node *actual_n)
{
  if(node->page_num == actual_n->page_num)
  {
    actual_n->offset = node->offset;
    return;
  }
  actualizeOffset(node, actual_n->next);
}

void actualizeFreePageFrames(Node *node, Node *page_frame)
{
  if(node == NULL) return;

  if(node->page_num == page_frame->page_num)
  {
    page_frame->free = 0;
    actualizeFreePageFrames(node->next, page_frame->next);
  }
  else
  {
    actualizeFreePageFrames(node, page_frame->next);
  }
}

//dividir la direccion entre el page size y eso me da

int proc_active_;
int *pids_;
List *pages_frames; //todas las paginas de la memoria fisica
List *dirs_spaces[__PROCESS_MAX__]; //espacios de direcciones de cada proceso
addr_t *free_list_;

//int *count_pages_by_proc;

//int *occupate;//indica si un page frame de la memoria esta siendo utilizado por un proceso
//addr_t *heaps;
//addr_t *stacks_pointers;
//addr_t *pages_frames;
//addr_t *free_page_frame;
//page_table *dir_spaces;


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv) 
{
  proc_active_ = -1;
  pids_ = (int *)malloc(__PROCESS_MAX__ * sizeof(int));
  pages_frames = newList();
  List *dir_space = (List *)malloc(__PROCESS_MAX__ * sizeof(List));

  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    pids_[i] = -1;;
    //dirs_spaces[i] = newList();
  }  

  for (size_t i = 0; i < m_size()/__PAGE_LENGTH__; i++)
  {
    Node *page = newNode(i * __PAGE_LENGTH__);
    Add(pages_frames, page);
  }

  for (size_t i = 0; i < m_size(); i++)
  {
    free_list_[i] = 0;
  } 
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out) 
{
  if(proc_active_ == -1) return 1;

  Node *node = newNode(dirs_spaces[proc_active_]->root->next->page_num);
  node = dirs_spaces[proc_active_]->root->next;

  if(suficentPageRec(dirs_spaces[proc_active_]->root->next, size, node))
  {
    if(node->offset + size < __PAGE_LENGTH__)
    {
      out->addr = node->page_num + node->offset;
      out->size = size;
      for (size_t i = node->page_num + node->offset; i < node->page_num + node->offset + size; i++)
      {
        free_list_[i] =1;
      }
      node->offset += size;
      actualizeOffset(node, dirs_spaces[proc_active_]->root);
      return 0;      
    }

    //asumiendo que siempre size <= __PAGE_LENGTH__
    out->addr = node->next->page_num + node->next->offset;
    out->size = size;
    node->offset = __PAGE_LENGTH__ - 1;
    actualizeOffset(node, dirs_spaces[proc_active_]->root);
    node->next->offset += size;
    actualizeOffset(node->next, dirs_spaces[proc_active_]->root);
    return 0;
  }

  dirs_spaces[proc_active_]->last->offset = __PAGE_LENGTH__ - 1;
  addr_t old_page_num = node->page_num;
  node = getPageFrame(pages_frames->root); //annadir una nueva pagina

  if(node->page_num == old_page_num) return 1;

  Add(dirs_spaces[proc_active_], node);
  out->addr = dirs_spaces[proc_active_]->last->page_num;
  out->size = size;
  dirs_spaces[proc_active_]->last->offset += size;
  return 0;  
}

// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr) 
{
  addr_t addr = -1;
  Node *node = newNode(addr);
  findAddrRec(ptr.addr, node, dirs_spaces[proc_active_]->root);
  if(node->page_num == addr) return 1;
  if(free_list_[ptr.addr] == 0) return 1;
  
  m_unset_owner(ptr.addr, ptr.addr + ptr.size);
  for (size_t i = ptr.addr; i < ptr.addr + ptr.size; i++)
  {
    free_list_[i] = 0;
  }

  node->offset -= ptr.size;
  actualizeOffset(node, dirs_spaces[proc_active_]->root);
  return 0;
}

// Agrega un elemento al stack
int m_pag_push(byte val, ptr_t *out) 
{
  if(dirs_spaces[proc_active_]->root->offset == 0) return 1;

  m_write (dirs_spaces[proc_active_]->root->page_num + dirs_spaces[proc_active_]->root->offset, val);

  out->addr = dirs_spaces[proc_active_]->root->offset;
  out->size = val;
  
  free_list_[dirs_spaces[proc_active_]->root->page_num + dirs_spaces[proc_active_]->root->offset] = 1;
  dirs_spaces[proc_active_]->root->offset --;
  return 0;  
}

// Quita un elemento del stack
int m_pag_pop(byte *out) 
{
  if(dirs_spaces[proc_active_]->root->offset == __PAGE_LENGTH__ - 1) return 1;
  
  dirs_spaces[proc_active_]->root->offset ++;
  *out = m_read(dirs_spaces[proc_active_]->root->page_num + dirs_spaces[proc_active_]->root->offset);
  free_list_[dirs_spaces[proc_active_]->root->page_num + dirs_spaces[proc_active_]->root->offset] = 0;
  return 0;  
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out) 
{
  addr_t addr_ = -1;
  Node *node = newNode(addr_);
  
  findAddrRec(addr, node, dirs_spaces[proc_active_]->root);
  if(node->page_num == addr_) return 1; //el addr no pertenece al proceso activo
  if(free_list_[node->page_num + addr] == 0) return 1;//no hay nada guardado en esa direccion
  *out = m_read(node->page_num + addr);
  return 0;   
}

// Almacena un valor en una dirección determinada
int m_pag_store(addr_t addr, byte val) 
{
  addr_t addr_ = -1;
  Node *node = newNode(addr_);
  
  findAddrRec(addr, node, dirs_spaces[proc_active_]->root);
  if(node->page_num == addr_) return 1; //el addr no pertenece al proceso activo
  if(free_list_[node->page_num + addr] == 0) return 1;//esa dir esta ocupada
  free_list_[node->page_num + addr] = 1;
  m_write(node->page_num + addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process) 
{
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] == process.pid)
    {
      proc_active_ = i;
      return;
    }
  }
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] != -1) continue;
    else 
    {
      proc_active_ = i;
      break;
    }   
  } 
  pids_[proc_active_] = process.pid;
  dirs_spaces[proc_active_] = newList();

  newDirSpaceRec(pages_frames->root, 0, __COUNT_INIT_PAGES__, dirs_spaces[proc_active_]); 
  //falta considerar el caso en el que no se pudo crear un nuevo dir space porque en la memoria fisica
  //no habian pages disponibles ???
  m_set_owner_rec(dirs_spaces[proc_active_]->root); 
  dirs_spaces[proc_active_]->root->offset = __PAGE_LENGTH__ - 1; 
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process) 
{
  for (int i = 0; i < __PROCESS_MAX__; i++)
  {
    if(pids_[i] != process.pid) continue;

    freeRec(free_list_, dirs_spaces[i]->root);
    
    m_unset_owner_rec(dirs_spaces[i]->root);

    actualizeFreePageFrames(dirs_spaces[i]->root, pages_frames->root);
    
    dirs_spaces[i] = NULL;
    pids_[i] = -1;
    return;    
  }  
}
