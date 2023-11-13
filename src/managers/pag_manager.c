#include "pag_manager.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef size_t addr_t;

#define max_mem 1000000
#define max_procs 1000
#define max_pages 100
#define stack_sz 1000
#define page_sz 1000

struct page_table
{
  int mk[max_pages][page_sz];
  int page_frame[max_pages];
  int used[max_pages];
} table[max_procs];

addr_t stack_pos[max_procs][stack_sz];
int stk_point[max_procs];
byte page_mem[max_mem];
int mem_alloc[max_mem];
int from_pid[max_mem];
int last_page = -1;
int curr_pid = 0;

size_t to_adress(int vpn, int offset)
{
  size_t ret = 0;
  ret = (ret | offset);
  vpn = (vpn << 10);
  ret = ret | vpn;
  return ret;
}

void decode(size_t address, int *vpn, int *offset)
{
  *offset = address & 0x3FF;
  *vpn = (address >> 10) & 0x3FFFFF;
}

void m_pag_init(int argc, char **argv)
{
  memset(mem_alloc, 0, sizeof(mem_alloc[0]) * max_mem);
  for (int i = 0; i < max_mem; i++)
  {
    from_pid[i] = -1;
  }

  memset(page_mem, 0, sizeof(page_mem[0]) * max_mem);
  memset(stk_point, 0, sizeof(stk_point[0]) * max_procs);

  for (int i = 0; i < max_procs; i++)
  {
    memset(stack_pos[i], 0, sizeof(stack_pos[i][0]) * stack_sz);
    memset(table[i].page_frame, 0, sizeof(table[i].page_frame[0]) * max_pages);
    memset(table[i].used, 0, sizeof(table[i].used[0]) * max_pages);
    memset(table[i].mk, 0, sizeof(table[i].mk[0][0]) * max_pages * page_sz);
  }
}

int m_pag_malloc(size_t size, ptr_t *out)
{
  int vpn = -1, offset = 0;
  for (int i = 0; i < max_pages; i++)
  {
    if (page_sz - table[curr_pid].used[i] >= (int)size)
    {
      vpn = i;
      offset = table[curr_pid].used[i];
      if (!table[curr_pid].used[i])
        table[curr_pid].page_frame[i] = ++last_page;
      table[curr_pid].used[i] += size;
      break;
    }
  }
  if (vpn < 0)
    return 1;

  for (int i = offset; i < (int)(offset + size); i++)
  {
    table[curr_pid].mk[vpn][i] = 1;
    from_pid[table[curr_pid].page_frame[vpn] * page_sz + i] = curr_pid;
  }
  out->addr = to_adress(vpn, offset);
  out->size = size;

  int pt = table[curr_pid].page_frame[vpn];
  int realp = pt * page_sz + offset;
  // printf("[Loc] %d %d %d : %d \n",curr_pid,vpn,offset,realp);
  return 0;
}

int mem_pos(addr_t addr)
{
  int vpn = 0, offset = 0;
  decode(addr, &vpn, &offset);
  int pt = table[curr_pid].page_frame[vpn];
  // printf("[Ask] %d %d %d : %d\n",curr_pid,vpn,offset,pt*page_sz+offset);
  if (from_pid[pt * page_sz + offset] != curr_pid)
    return -1;
  return pt * page_sz + offset;
}

int m_pag_store(addr_t addr, byte val)
{
  int pos = mem_pos(addr);
  if (pos < 0)
    return 1;
  page_mem[pos] = val;
  return 0;
}

int m_pag_load(addr_t addr, byte *out)
{
  int pos = mem_pos(addr);
  if (pos < 0)
    return 1;
  *out = page_mem[pos];
  return 0;
}

void m_pag_on_ctx_switch(process_t process)
{
  curr_pid = process.pid;
}

int m_pag_push(byte val, ptr_t *out)
{
  int ret = m_pag_malloc(1, out);
  if (ret)
    return 1;
  m_pag_store(out->addr, val);
  stack_pos[curr_pid][stk_point[curr_pid]] = out->addr;
  stk_point[curr_pid]++;
  return 0;
}

int m_pag_pop(byte *out)
{
  if (stk_point[curr_pid] == 0)
    return 1;
  stk_point[curr_pid]--;
  addr_t addr = stack_pos[curr_pid][stk_point[curr_pid]];
  return m_pag_load(addr, out);
}

void m_pag_on_end_process(process_t process)
{
  int pid = process.pid;
  mem_alloc[pid] = page_mem[pid] = stk_point[pid] = 0;
  for (int i = 0; i < max_pages; i++)
  {
    if (table[pid].used[i])
      for (int j = 0; j < page_sz; j++)
      {
        from_pid[table[pid].page_frame[i] * page_sz + j] = -1;
      }
  }
  memset(stack_pos[pid], 0, sizeof(stack_pos[pid][0]) * stack_sz);
  memset(table[pid].page_frame, 0, sizeof(table[pid].page_frame[0]) * max_pages);
  memset(table[pid].used, 0, sizeof(table[pid].used[0]) * max_pages);
  memset(table[pid].mk, 0, sizeof(table[pid].mk[0][0]) * max_pages * page_sz);
}

int m_pag_free(ptr_t ptr)
{
  m_pag_store(ptr.addr, 0);
  int vpn = 0, offset = 0;
  decode(ptr.addr, &vpn, &offset);

  for (int i = offset; i < (int)(offset + ptr.size); i++)
  {
    table[curr_pid].mk[vpn][i] = 0;
  }
  for (int i = table[curr_pid].used[vpn]; i >= 0; i--)
  {
    if (table[curr_pid].mk[vpn][i])
      break;
    else
      table[curr_pid].used[vpn] = i;
  }
  return 0;
}