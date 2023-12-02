#include "utils.h"

#include <stdlib.h>

program_t new_program(char *name, size_t size) {
  return (program_t){name, size};
}

process_t new_process(int pid, program_t *program) {
  return (process_t){pid, program,-1};
}

node_t init_linked_list(int addr_,int len_)
{
  return (node_t){addr_,len_,NULL};
}

void add_node(node_t *root,int addr_,int len_)
{
  node_t *current = root;
  while(current->next != NULL && current->next->addr < addr_)
  {
    current = current->next;
  }
  node_t *aux = current->next;
  current->next = malloc(sizeof(node_t));
  current->next->addr = addr_;
  current->next->len = len_;
  current->next->next = aux;
}

void upd_linked_list(node_t *root)
{
  node_t *current = root;
  int new_len = 0;
  while(current->next != NULL)
  {
    if(current->addr + current->len == current->next->addr) 
    {
      current->len = current->len + current->next->len;
      node_t *aux = current->next;
      current->next = aux->next;
      free(aux);
    }
    current = current->next;
  }
}

void delete_node(node_t *root,int addr_)
{
  node_t *current = root;
  while(current->next != NULL && current->next->addr != addr_)
  {
    current = current->next;
  }
  if(current->next == NULL)
  {
    printf("Intentas ocupar un espacio de memoria que no es libre/n");
    return;
  }
  node_t *aux = current->next;
  current->next = aux->next;
  free(aux);
}
