#ifndef DATA_STRUCTURES

#define MAX_NUMBER_PROGRAMS 100
#define PROGRAM_SIZE 400

typedef struct free_list_node
{
    unsigned long start;
    unsigned long size;
    struct free_list_node *next;
} free_list_node;

typedef struct free_list
{
    free_list_node *head;
    unsigned long size;
} free_list;

typedef struct pcb
{
    int pid;
    unsigned long code_start;
    unsigned long code_end;
    unsigned long heap_start;
    unsigned long heap_end;
    unsigned long stack_start;
    unsigned long stack_pointer;
    unsigned long stack_end;
    unsigned long p_address;
    int *page_table;
    free_list *fl_heap;
} pcb;

void pcb_init(pcb *process, unsigned long start, unsigned long end, unsigned long code_size, int pid);
free_list *create_fl(unsigned long size);
void free_list_free(free_list *f);
int can_insert(free_list *l, unsigned long size);
int is_recoverable(free_list *l, unsigned long start, unsigned long size);
void recover_space(free_list *l, unsigned long start, unsigned long size);
unsigned long request_space(free_list *l, unsigned long size);
int is_occupied(free_list *l, unsigned long pos);
void print_free_list(free_list *A);
void print_pcb(pcb *A);

#endif