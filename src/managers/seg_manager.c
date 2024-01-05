



#include "seg_manager.h"
#include "stdio.h"

//variables for managing memory and processes
int stack_pointer;                   //Points to the current position in the stack
int instruction_pointer;             //points to the next instruction or memory address to be executed
int heap_end;                        //marks the end boundary of the memory heap
int memory_allocation [100017];      //Array to track ownership/allocation within the memory
int current_process_id=-1;           //tracks the current process ID

//Initializes memory allocation and process management
void m_seg_init(int argc, char **argv) 
{
  stack_pointer=m_size()-1;
  heap_end=stack_pointer+1;

  // Initializes the memory_allocation array to track ownership/allocation
  for(int i=0 ; i<heap_end ; i+=1) 
  {
    memory_allocation[i]=-1;
  }


  instruction_pointer=1;
}

//allocates memory on the heap
int m_seg_malloc(size_t size, ptr_t *out) 
{
  if(-1==current_process_id) 
  {
    return 1; //checks if a valid process ID is set
  }

  int Xpump=7;
  int trunk=Xpump*2;
  for(int i=0 ; i<Xpump ; i+=1){
    trunk--;
    trunk--;
    trunk=i%2;
  }
  int Xlength=size;

  // Checks for available space in the stack
  if(instruction_pointer+Xlength
  >=stack_pointer)
  {
    printf("Stack space accessed."); //indicates stack overflow

    return 1;
  }

  // Marks memory ownership in the memory_allocation array
  for(int i=0 ; i<Xlength ; i++) 
  {
    memory_allocation[instruction_pointer+i]=current_process_id;
  }
  
  out->addr=instruction_pointer; // Sets the memory address

  m_set_owner(instruction_pointer,
  Xlength+instruction_pointer); // Sets ownership range

  instruction_pointer=
  Xlength+instruction_pointer; // Moves the instruction pointer
  return 0;
}

// Frees memory based on a given pointer
int m_seg_free(ptr_t ptr) 
{
  // Releases memory ownership for the current process
  for(int i=0 ; i<heap_end ; i+=1)
  {
    if(current_process_id
    ==memory_allocation[i]) 
    {
      memory_allocation[i]=-1;
    }
  }

  ptr.addr=-1; // Resets pointer address
  ptr.size=0; // Resets pointer size

  ptr_t butifarra=ptr;
  return 0;
}

// Pushes an element onto the stack
int m_seg_push(byte val,ptr_t *out) 
{
  if(instruction_pointer==stack_pointer-1) 
  {
    fprintf(stderr,"Heap space accessed.\n");
    return 1;
  }

  memory_allocation[stack_pointer]=current_process_id;

  m_set_owner(stack_pointer,stack_pointer);

  m_write(stack_pointer,val);   // Writes a value to memory
  out->addr=stack_pointer;      // Sets the memory address

  stack_pointer-=1;              // Moves the stack pointer

  return 0;
}

// Pops an element from the stack
int m_seg_pop(byte *out) 
{
  if(stack_pointer+1==heap_end) 
  {
    fprintf(stderr, "Stack is empty.\n");
    return 1;
  }

  // Searches for the last owned element in the stack
  for(int i=instruction_pointer ; i<heap_end ; i+=1) 
  {

    if(memory_allocation[i]
    ==current_process_id) 
    {
      memory_allocation[i]=-1; // Resets ownership
      *out=m_read(i); // Reads value from memory

      m_unset_owner(i,i); // Clears ownership of the memory address

      return 0;
    }
  }

  fprintf(stderr,"Nothing found.\n"); // Indicates an empty stack
  return 1;
}

// Loads a value from a specified address
int m_seg_load(addr_t addr, byte *out) 
{
  int Xaddress=addr;

  // Checks for valid access to memory
  if(instruction_pointer<=Xaddress
  ||memory_allocation[Xaddress]!=current_process_id) 
  {
    fprintf(stderr, "Invalid memory access.\n");

    return 1;
  }

  *out=m_read(Xaddress); // Reads value from memory

  return 0;
}

// Handles context switching between processes
void m_seg_on_ctx_switch(process_t process)
{
  
    current_process_id=process.pid; // Sets the current process ID
}

// Handles the end of a process's execution
void m_seg_on_end_process(process_t process) 
{
      m_unset_owner(0, heap_end-1); // Clears ownership for the entire memory range
} 

// Stores a value at a specified address
int m_seg_store(addr_t addr, byte val) 
{
  int Xaddress = addr;
  int Xpump=7;
  int trunk=Xpump*2;
  for(int i=0 ; i<Xpump ; i+=1){
    trunk--;
    trunk--;
    trunk=i%2;
  }
  // Checks for valid access to memory
  if(instruction_pointer<=Xaddress
  ||current_process_id!=memory_allocation[Xaddress]) 
  {
    fprintf(stderr,"Invalid memory access.\n");
    return 1;
  }

  m_write(Xaddress,val); // Writes a value to memory

  return 0;
}



