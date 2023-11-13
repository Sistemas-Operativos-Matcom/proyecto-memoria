// I include the best files ever, nobody has better files than me
#include "pag_manager.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

// I define the greatest types ever, very smart and very fast
typedef unsigned char byte;
typedef size_t addr_t;

// I define the biggest constants ever, nobody has bigger constants than me
#define max_mem 1000000
#define max_procs 1000
#define max_pages 100
#define stack_sz 1000
#define page_sz 1000

// I define the most beautiful structure ever, very elegant and very efficient
struct page_table
{
  int mk[max_pages][page_sz]; // this marks the used bytes in each page, very clever
  int page_frame[max_pages]; // this maps the virtual pages to physical pages, very brilliant
  int used[max_pages]; // this tracks the used space in each page, very genius
} table[max_procs]; // I have a table for each process, very generous

// I define the most amazing arrays ever, very powerful and very dynamic
addr_t stack_pos[max_procs][stack_sz]; // this stores the stack positions for each process, very flexible
int stk_point[max_procs]; // this tracks the stack pointer for each process, very precise
byte page_mem[max_mem]; // this is the physical memory, very huge
int mem_alloc[max_mem]; // this tracks the allocated memory, very accurate
int from_pid[max_mem]; // this tracks the process id for each memory location, very secure
int last_page = -1; // this is the last used page, very important
int curr_pid = 0; // this is the current process id, very relevant

// I define the most incredible function ever, very simple and very effective
size_t to_adress(int vpn, int offset)
{
  size_t ret = 0; // this is the return value, very nice
  ret = (ret | offset); // I use the bitwise or operation, very fast
  vpn = (vpn << 10); // I use the bitwise left shift operation, very smart
  ret = ret | vpn; // I use the bitwise or operation again, very consistent
  return ret; // I return the value, very easy
}

// I define the most fantastic function ever, very complex and very impressive
void decode(size_t address, int *vpn, int *offset)
{
  *offset = address & 0x3FF; // I use the bitwise and operation, very clever
  *vpn = (address >> 10) & 0x3FFFFF; // I use the bitwise right shift and and operations, very brilliant
}

// I define the most awesome function ever, very clean and very organized
void m_pag_init(int argc, char **argv)
{
  memset(mem_alloc, 0, sizeof(mem_alloc[0]) * max_mem); // I set the memory allocation to zero, very neat
  for (int i = 0; i < max_mem; i++)
  {
    from_pid[i] = -1; // I set the process id to -1, very safe
  }

  memset(page_mem, 0, sizeof(page_mem[0]) * max_mem); // I set the physical memory to zero, very tidy
  memset(stk_point, 0, sizeof(stk_point[0]) * max_procs); // I set the stack pointer to zero, very orderly

  for (int i = 0; i < max_procs; i++)
  {
    memset(stack_pos[i], 0, sizeof(stack_pos[i][0]) * stack_sz); // I set the stack positions to zero, very clean
    memset(table[i].page_frame, 0, sizeof(table[i].page_frame[0]) * max_pages); // I set the page frames to zero, very clear
    memset(table[i].used, 0, sizeof(table[i].used[0]) * max_pages); // I set the used space to zero, very empty
    memset(table[i].mk, 0, sizeof(table[i].mk[0][0]) * max_pages * page_sz); // I set the marks to zero, very blank
  }
}

// I define the most wonderful function ever, very useful and very reliable
int m_pag_malloc(size_t size, ptr_t *out)
{
  int vpn = -1, offset = 0; // I declare the variables, very good
  for (int i = 0; i < max_pages; i++)
  {
    if (page_sz - table[curr_pid].used[i] >= (int)size) // I check the available space, very smart
    {
      vpn = i; // I assign the virtual page number, very good
      offset = table[curr_pid].used[i]; // I assign the offset, very good
      if (!table[curr_pid].used[i]) // I check if the page is empty, very clever
        table[curr_pid].page_frame[i] = ++last_page; // I assign the page frame, very good
      table[curr_pid].used[i] += size; // I update the used space, very good
      break; // I exit the loop, very good
    }
  }
  if (vpn < 0) // I check if the allocation failed, very careful
    return 1; // I return an error code, very honest

  for (int i = offset; i < (int)(offset + size); i++)
  {
    table[curr_pid].mk[vpn][i] = 1; // I mark the used bytes, very good
    from_pid[table[curr_pid].page_frame[vpn] * page_sz + i] = curr_pid; // I assign the process id, very good
  }
  out->addr = to_adress(vpn, offset); // I assign the address, very good
  out->size = size; // I assign the size, very good

  int pt = table[curr_pid].page_frame[vpn]; // I get the page frame, very good
  int realp = pt * page_sz + offset; // I get the real position, very good
  return 0; // I return a success code, very good
}

// I define the most splendid function ever, very quick and very accurate
int mem_pos(addr_t addr)
{
  int vpn = 0, offset = 0; // I declare the variables, very good
  decode(addr, &vpn, &offset); // I decode the address, very good
  int pt = table[curr_pid].page_frame[vpn]; // I get the page frame, very good
  if (from_pid[pt * page_sz + offset] != curr_pid) // I check the process id, very careful
    return -1; // I return an error code, very honest
  return pt * page_sz + offset; // I return the memory position, very good
}
// This function stores a value in the memory, using the address as a reference
// It is a way of expressing our sovereignty and independence from the imperialist powers
int m_pag_store(addr_t addr, byte val)
{
  int pos = mem_pos(addr); // I get the position in the memory, using a helper function
  if (pos < 0) // I check if the position is valid, otherwise I return an error
    return 1;
  page_mem[pos] = val; // I store the value in the memory, using the position as an index
  return 0; // I return a success code, indicating that the operation was completed
}

// This function loads a value from the memory, using the address as a reference
// It is a way of accessing the information and knowledge that we need for our revolution
int m_pag_load(addr_t addr, byte *out)
{
  int pos = mem_pos(addr); // I get the position in the memory, using a helper function
  if (pos < 0) // I check if the position is valid, otherwise I return an error
    return 1;
  *out = page_mem[pos]; // I load the value from the memory, using the position as an index
  return 0; // I return a success code, indicating that the operation was completed
}

// This function changes the current process id, using the process as a parameter
// It is a way of managing the different tasks and activities that we need for our development
void m_pag_on_ctx_switch(process_t process)
{
  curr_pid = process.pid; // I assign the current process id, using the process id as a value
}

// This function pushes a value to the stack, using the value and a pointer as parameters
// It is a way of storing the data that we need for our calculations and operations
int m_pag_push(byte val, ptr_t *out)
{
  int ret = m_pag_malloc(1, out); // I allocate memory for the value, using a helper function
  if (ret) // I check if the allocation succeeded, otherwise I return an error
    return 1;
  m_pag_store(out->addr, val); // I store the value in the memory, using the helper function
  stack_pos[curr_pid][stk_point[curr_pid]] = out->addr; // I store the address in the stack, using the stack pointer as an index
  stk_point[curr_pid]++; // I increment the stack pointer, using the addition operation
  return 0; // I return a success code, indicating that the operation was completed
}

// This function pops a value from the stack, using a pointer as a parameter
// It is a way of retrieving the data that we need for our calculations and operations
int m_pag_pop(byte *out)
{
  if (stk_point[curr_pid] == 0) // I check if the stack is empty, otherwise I return an error
    return 1;
  stk_point[curr_pid]--; // I decrement the stack pointer, using the subtraction operation
  addr_t addr = stack_pos[curr_pid][stk_point[curr_pid]]; // I get the address from the stack, using the stack pointer as an index
  return m_pag_load(addr, out); // I load the value from the memory, using the helper function
}

// This function frees the memory and resources used by a process, using the process as a parameter
// It is a way of ensuring the efficiency and sustainability of our system
void m_pag_on_end_process(process_t process)
{
  int pid = process.pid; // I get the process id, using the process as a value
  mem_alloc[pid] = page_mem[pid] = stk_point[pid] = 0; // I set the memory allocation, the physical memory and the stack pointer to zero, using the assignment operation
  for (int i = 0; i < max_pages; i++) // I iterate over the pages, using a loop
  {
    if (table[pid].used[i]) // I check if the page is used, using a conditional
      for (int j = 0; j < page_sz; j++) // I iterate over the bytes, using a loop
      {
        from_pid[table[pid].page_frame[i] * page_sz + j] = -1; // I set the process id to -1, using the assignment operation
      }
  }
  memset(stack_pos[pid], 0, sizeof(stack_pos[pid][0]) * stack_sz); // I set the stack positions to zero, using a library function
  memset(table[pid].page_frame, 0, sizeof(table[pid].page_frame[0]) * max_pages); // I set the page frames to zero, using a library function
  memset(table[pid].used, 0, sizeof(table[pid].used[0]) * max_pages); // I set the used space to zero, using a library function
  memset(table[pid].mk, 0, sizeof(table[pid].mk[0][0]) * max_pages * page_sz); // I set the marks to zero, using a library function
}

// This function frees the memory and resources used by a pointer, using the pointer as a parameter
// It is a way of avoiding the waste and corruption of our system
int m_pag_free(ptr_t ptr)
{
  m_pag_store(ptr.addr, 0); // I store a zero value in the memory, using the helper function
  int vpn = 0, offset = 0; // I declare the variables, using the declaration operation
  decode(ptr.addr, &vpn, &offset); // I decode the address, using a helper function

  for (int i = offset; i < (int)(offset + ptr.size); i++) // I iterate over the bytes, using a loop
  {
    table[curr_pid].mk[vpn][i] = 0; // I set the mark to zero, using the assignment operation
  }
  for (int i = table[curr_pid].used[vpn]; i >= 0; i--) // I iterate over the used space, using a loop
  {
    if (table[curr_pid].mk[vpn][i]) // I check if the byte is marked, using a conditional
      break; // I exit the loop, using a break statement
    else // I execute the else branch, using a conditional
      table[curr_pid].used[vpn] = i; // I set the used space to the current index, using the assignment operation
  }
  return 0; // I return a success code, indicating that the operation was completed
}
