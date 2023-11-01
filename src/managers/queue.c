
#include "queue.h" 
#include <stdio.h>
#include <stdlib.h> 




// Utility function to initialize a queue
struct queue* new_queue(int size)
{
	struct queue *q = NULL;
	q = (struct queue*)malloc(sizeof(struct queue));

	q->items = (int*)malloc(size * sizeof(int));
	q->proc_time = (int*)malloc(size * sizeof(int));
	q->max_size = size;
	q->front = 0;
	q->last = -1;
	q->size = 0;

	return q;
}

// Utility function to return the size of the queue
int size(struct queue *q) {
	return q->size;
}

// Utility function to check if the queue is emqy or not
int is_empty(struct queue *q) {
	return !size(q);
}

// Utility function to return the front element of the queue
int front(struct queue *q)
{
	if (is_empty(q))
	{
		printf("Underflow by front\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	return q->items[q->last];
}

int add_slide_time(struct queue *q) {
  if (is_empty(q))
	{
		printf("Underflow by slide time\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	q->proc_time[q->front] += 10;
  return q->proc_time[q->front];
}

// Utility function to add an element `x` to the queue
void enqueue(struct queue *q, int x)
{
	if (size(q) == q->max_size)
	{
		printf("Overflow by enqueue\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Inserting %d\t", x);

	q->last = (q->last + 1) % q->max_size;	// circular queue
	q->items[q->last] = x;
  q->proc_time[q->last] = 0;
	q->size++;

	// printf("front = %d, last = %d\n", q->front, q->last);
}

// Utility function to dequeue the front element
void dequeue(struct queue *q)
{
	if (is_empty(q))	// front == last
	{
		printf("Underflow by dequeue\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Removing %d\t", front(q));

	q->front = (q->front + 1) % q->max_size;  // circular queue
	q->size--;

	// printf("front = %d, last = %d\n", q->front, q->last);
}