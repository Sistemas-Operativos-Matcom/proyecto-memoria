
#include "stack.h" 
#include <stdio.h>
#include <stdlib.h> 




// Utility function to initialize a stack
struct stack* new_stack(int size)
{
	struct stack *q = NULL;
	q = (struct stack*)malloc(sizeof(struct stack));

	q->items = (int*)malloc(size * sizeof(int));
	q->proc_time = (int*)malloc(size * sizeof(int));
	q->max_size = size;
	q->front = 0;
	q->last = -1;
	q->size = 0;

	return q;
}

// Utility function to return the size of the stack
int size(struct stack *q) {
	return q->size;
}

// Utility function to check if the stack is emqy or not
int is_empty(struct stack *q) {
	return !size(q);
}

// Utility function to return the front element of the stack
int front(struct stack *q)
{
	if (is_empty(q))
	{
		printf("Underflow by front\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	return q->items[q->last];
}

int add_slide_time(struct stack *q) {
  if (is_empty(q))
	{
		printf("Underflow by slide time\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	q->proc_time[q->front] += 10;
  return q->proc_time[q->front];
}

// Utility function to add an element `x` to the stack
void enstack(struct stack *q, int x)
{
	if (size(q) == q->max_size)
	{
		printf("Overflow by enstack\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Inserting %d\t", x);

	q->last = (q->last + 1) % q->max_size;	// circular stack
	q->items[q->last] = x;
  q->proc_time[q->last] = 0;
	q->size++;

	// printf("front = %d, last = %d\n", q->front, q->last);
}

// Utility function to destack the front element
void destack(struct stack *q)
{
	if (is_empty(q))	// front == last
	{
		printf("Underflow by destack\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Removing %d\t", front(q));
	q->last = (q->last - 1) % q->max_size;  // circular stack
	// printf("q %i \n", q->items[q->front]);
	q->size--;

	// printf("front = %d, last = %d\n", q->front, q->last);
}