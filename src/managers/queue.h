#ifndef B633082A_609D_42C5_B958_6A8DF8A8F662
#define B633082A_609D_42C5_B958_6A8DF8A8F662

// Data structure to represent a queue
struct queue
{
	int *items; 	// array to store queue elements
  int *proc_time; // array asociado al items con sus tiempo que llevo ejecutando el proceso
	int max_size;	// maximum capacity of the queue
	int front;  	// front points to the front element in the queue (if any)
	int last;   	// last points to the last element in the queue
	int size;   	// current capacity of the queue
};


struct queue* new_queue(int size);
int size(struct queue *q);
int is_empty(struct queue *q);
int front(struct queue *q);
int add_slide_time(struct queue *q);
void enqueue(struct queue *q, int x);
void dequeue(struct queue *q);

#endif /* B633082A_609D_42C5_B958_6A8DF8A8F662 */
