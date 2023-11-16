#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "aux_structs.h"


void Free(space_t *head, addr_t address) {
  
    if (head == NULL) {
        return;
    }
  
  while (head != NULL)
    {
        if (head->address == address)
        {
            head->prev->next = head->next;
            head->next->prev = head->prev;
        
            free(head);
            break;
        }
    }
}
