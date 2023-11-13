// Include the header file for the bnb manager
#include "bnb_manager.h"

// Include the standard input/output library
#include "stdio.h"

// Declare some static variables to store the current process id, address, and pointers to the processes' addresses and virtual memory blocks
static int curr_pid;
static addr_t curr_addr;
static addr_t *procs_addr;
static mem_block_t *v_mem;

// Define some constants for the block size, code size, and a macro to convert bytes to kilobytes
#define block_sz 1024
#define code_sz 1
#define kb(size) ((size) / block_sz)

// Define a function to initialise the memory manager with the given arguments
void m_bnb_init(int argc, char **argv)
{
  // If the virtual memory is not null, free it and set it to null
  if (v_mem != NULL)
  {
    free(v_mem);
    v_mem = NULL;
  }

  // If the processes' addresses are not null, free them and set them to null
  if (procs_addr != NULL)
  {
    free(procs_addr);
    procs_addr = NULL;
  }

  // Calculate the number of blocks needed for the memory size
  size_t block_count = kb(m_size());

  // Allocate memory for the virtual memory blocks and the processes' addresses
  v_mem = (mem_block_t *)malloc(sizeof(mem_block_t) * block_count);
  procs_addr = (size_t *)malloc(sizeof(size_t) * block_count);
  // Set the current address to zero
  curr_addr = 0;

  // Loop through the blocks and initialise their fields
  for (size_t i = 0, start = 0; i < block_count; i++, start += block_sz)
  {
    // Get a pointer to the current block
    mem_block_t *curr_block = &v_mem[i];
    // Set the heap to the start of the block plus the code size
    curr_block->heap = start + code_sz;
    // Set the stack to the end of the block minus one
    curr_block->stack = start + block_sz - 1;
    // Set the start and end of the block
    curr_block->st = start + code_sz;
    curr_block->end = start + block_sz - 1;
    // Set the size, usage, and owner of the block to zero
    curr_block->sz = 0;
    curr_block->on_use = 0;
    curr_block->user = NO_ONWER;
  }
}

// Define a function to allocate memory for a given size and return a pointer to it
int m_bnb_malloc(size_t size, ptr_t *out)
{
  // Loop through the blocks and find the first one that is not in use
  for (size_t i = 0; i < kb(m_size()); i++)
  {
    if (!v_mem[i].on_use)
    {
      // Calculate the offset of the block
      size_t offset = i * block_sz;
      // Set the owner of the block to the current process
      m_set_owner(offset + code_sz, offset + block_sz - 1);
      // Store the address of the block in the processes' addresses array
      procs_addr[curr_pid] = i;
      // Set the current address to the block index
      curr_addr = i;
      // Mark the block as in use and set its user and size
      v_mem[i].on_use = 1;
      v_mem[i].user = curr_pid;
      v_mem[i].sz = size;

      // Set the output pointer to the start of the block plus the code size and set its size to one
      out->addr = offset + code_sz;
      out->size = 1;
      // Return zero to indicate success
      return 0;
    }
  }
  // Return one to indicate failure
  return 1;
}

// Define a function to free a given pointer
int m_bnb_free(ptr_t ptr)
{
  // Get the start and end of the current block
  size_t start = v_mem[curr_addr].st;
  size_t end = v_mem[curr_addr].end;

  // Check if the pointer is within the block boundaries
  if (ptr.addr >= start && ptr.addr + ptr.size < end)
  {
    // Unset the owner of the pointer
    m_unset_owner(ptr.addr, ptr.addr + ptr.size - 1);
    // Decrease the size of the block by the pointer size
    v_mem[curr_addr].sz -= ptr.size;
    // Return zero to indicate success
    return 0;
  }

  // Return one to indicate failure
  return 1;
}

// Define a function to push a byte value to the stack and return a pointer to it
int m_bnb_push(byte val, ptr_t *out)
{
  // Check if the stack pointer is not overlapping with the heap pointer
  if (v_mem[curr_addr].stack - 1 <= v_mem[curr_addr].heap)
  {
    // Return one to indicate failure
    return 1;
  }

  // Write the value to the stack pointer
  m_write(v_mem[curr_addr].stack, val);
  // Decrement the stack pointer
  v_mem[curr_addr].stack--;
  // Set the output pointer to the stack pointer and return zero to indicate success
  out->addr = v_mem[curr_addr].stack;
  return 0;
}

// Define a function to pop a byte value from the stack and return it
int m_bnb_pop(byte *out)
{
  // Get the stack top, the start and the end of the block
  addr_t stack_top = v_mem[curr_addr].stack + 1;
  addr_t block_st = v_mem[curr_addr].st;
  addr_t block_end = v_mem[curr_addr].end;

  // Check if the stack top is within the block boundaries
  if (block_st + block_end <= stack_top)
  {
    // Return one to indicate failure
    return 1;
  }

  // Read the value from the stack top and store it in the output parameter
  *out = m_read(stack_top);
  // Increment the stack pointer and return zero to indicate success
  v_mem[curr_addr].stack++;
  return 0;
}

// Define a function to load a byte value from a given address and return it
int m_bnb_load(addr_t addr, byte *out)
{
  // Get the start and end of the block
  addr_t start = v_mem[curr_addr].st;
  addr_t end = v_mem[curr_addr].end;

  // Check if the address is within the block boundaries
  if (addr >= start && addr < end)
  {
    // Read the value from the address and store it in the output parameter
    *out = m_read(addr);
    // Return zero to indicate success
    return 0;
  }

  // Return one to indicate failure
  return 1;
}

// Define a function to store a byte value to a given address
int m_bnb_store(addr_t addr, byte val)
{
  // Get the start address and the size of the block
  addr_t star_addr = v_mem[curr_addr].st;
  addr_t curr_sz = v_mem[curr_addr].sz;

  // Check if the address is within the allocated memory of the block
  if (addr >= star_addr && addr < star_addr + curr_sz)
  {
    // Write the value to the address
    m_write(addr, val);
    // Return zero to indicate success
    return 0;
  }

  // Return one to indicate failure
  return 1;
}

// Define a function to handle the context switch of the processes
void m_bnb_on_ctx_switch(process_t process)
{
  // Set the current process id to the given process id
  curr_pid = process.pid;
  // Set the current address to the address of the given process
  curr_addr = procs_addr[process.pid];
}

// Define a function to handle the end of a process
void m_bnb_on_end_process(process_t process)
{
  // Get the address of the process
  addr_t addr = procs_addr[process.pid];
  // Unset the owner of the block
  m_unset_owner(v_mem[addr].st, v_mem[addr].end);
  // Mark the block as not in use and set its user to no owner
  v_mem[addr].on_use = 0;
  v_mem[addr].user = NO_ONWER;
  // Set the size of the block to zero
  v_mem[addr].sz = 0;
  // Reset the heap and stack pointers to the start and end of the block
  v_mem[addr].heap = v_mem[addr].st;
  v_mem[addr].stack = v_mem[addr].end;
}
