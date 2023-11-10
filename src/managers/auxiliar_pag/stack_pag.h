#include <stdio.h>
#include <stdlib.h>

typedef struct Stack
{
    int stack_size;
    size_t *stack_s;
    int top;
} stack;
/* typedef struct Dupla
{
    // representa la página en el proceso
    size_t pag;
    // representa la posición dentro de la página
    size_t pos_pag;
} dupla;
 */
stack *Init_s_pag(int size);

int Push_s_pag(stack *s, size_t addr);
size_t Pop_s_pag(stack *s);
void Free_s_pag(stack *s);
