#ifndef B633082A_609D_42C5_B958_6A8DF8A8F662
#define B633082A_609D_42C5_B958_6A8DF8A8F662

// Data structure to represent a stack
struct stack
{
	int *items; 	// array to store stack elements
  int *proc_time; // array asociado al items con sus tiempo que llevo ejecutando el proceso
	int max_size;	// maximum capacity of the stack
	int front;  	// front points to the front element in the stack (if any)
	int last;   	// last points to the last element in the stack
	int size;   	// current capacity of the stack
};


struct stack* new_stack(int size);
int size(struct stack *q);
int is_empty(struct stack *q);
int front(struct stack *q);
int add_slide_time(struct stack *q);
void enstack(struct stack *q, int x);
void destack(struct stack *q);

#endif /* B633082A_609D_42C5_B958_6A8DF8A8F662 */
