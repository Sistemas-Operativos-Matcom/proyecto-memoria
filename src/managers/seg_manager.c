#include "seg_manager.h"
#include "time.h"
#include "stdio.h"

#include "../memory.h"

#ifndef VAL_CODE
#define VAL_CODE printf("OsvaldoMoreno_Val46")
#define fori(base, bound) for (size_t i = (size_t)base; i < (size_t)base + bound; i++)
#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b
#define clamp(a, b, x) (x > b) ? b : (x < a) ? a \
                                             : x
#define MAX_PROC_COUNT 20
#endif

/// 🚫 PROGRAMANDO CON SUENNO 🚫 ///

int *seg_pids;
addr_t *seg_base;
int seg_cur_ipid;

int seg_offset;

addr_t *seg_stack_bound;
addr_t *seg_heap;
addr_t *seg_stack;
int *seg_virtual_memory; // pid-> used -1-> not used

const int DEFAULT_STACK = 50;
// functions
// PIDS
int seg_find_pid(int pid)
{
  fori(0, MAX_PROC_COUNT) if (seg_pids[i] == pid) return i;
  return -1;
}

int seg_free_pid()
{
  return seg_find_pid(-1);
}

// ADDRESSES
addr_t off(addr_t x)
{
  return x & ((1 << seg_offset) - 1);
}
addr_t seg_pa(addr_t va)
{
  int is_heap = (va >> seg_offset) == 0;
  va = off(va);
  return seg_base[seg_cur_ipid] + (is_heap ? va : -va);
}

int seg_alloc_free_memory(addr_t paddress, int while_x, int alloc)
{
  fori(paddress, while_x)
  {
    if ((alloc && seg_virtual_memory[i] >= 0) || (!alloc && seg_virtual_memory[i] < 0))
    {
      printf("ERROR in memory manager Allocated: %d,  Onwer: %d   Addr: %ld\n", alloc, seg_virtual_memory[i], i);
      // return 1;
    }

    seg_virtual_memory[i] = alloc ? seg_pids[seg_cur_ipid] : -1;
  }

  if (alloc)
    m_set_owner(paddress, paddress + while_x - 1);

  else
    m_unset_owner(paddress, paddress + while_x - 1);

  return 0;
}

// Segmentation
int default_size_of_arrays = 100;

// Buddy allocation
int *seg_buddy_alloc_l;
int *seg_buddy_alloc_r;
int seg_buddy_alloc_count;
int *is_buddy_alloc; // pid allocated

int seg_size_buddy(int free_list_index)
{
  return seg_buddy_alloc_r[free_list_index] - seg_buddy_alloc_l[free_list_index];
}

void print_state()
{
  fori(0, seg_buddy_alloc_count)
  {
    printf("Segment: %d %d Use: %d\n", seg_buddy_alloc_l[i], seg_buddy_alloc_r[i], is_buddy_alloc[i]);
  }
}

void seg_buddy_join(int a, int b)
{
  seg_buddy_alloc_r[a] = seg_buddy_alloc_r[b];

  while (b < seg_buddy_alloc_count)
  {
    seg_buddy_alloc_l[b] = seg_buddy_alloc_l[b + 1];
    seg_buddy_alloc_r[b] = seg_buddy_alloc_r[b + 1];
    is_buddy_alloc[b] = is_buddy_alloc[b + 1];
    b++;
  }
  seg_buddy_alloc_count--;
}

void seg_buddy_update_free_list()
{
  int temp = -1;

  while (temp != seg_buddy_alloc_count)
  {
    temp = seg_buddy_alloc_count;

    for (int i = 0; i < seg_buddy_alloc_count - 1; i++)
    {
      if (seg_size_buddy(i) == seg_size_buddy(i + 1))
      {
        if (is_buddy_alloc[i] < 0 && is_buddy_alloc[i + 1] < 0)
        {
          seg_buddy_join(i, i + 1);
          i -= 1;
        }
        else
        {
          i += 1;
        }
      }
    }
  }
}

void seg_swap(int *l1, int *l2)
{
  int temp = *l1;
  *l1 = *l2;
  *l2 = temp;
}

void seg_buddy_propagation(int index)
{
  int mid = (seg_buddy_alloc_l[index] + seg_buddy_alloc_r[index]) / 2;

  int templ = mid;
  int tempr = seg_buddy_alloc_r[index];
  int temprA = -1;

  seg_buddy_alloc_r[index] = mid;
  is_buddy_alloc[index] = -1;

  index++;

  while (index <= seg_buddy_alloc_count)
  {
    seg_swap(&seg_buddy_alloc_l[index], &templ);
    seg_swap(&seg_buddy_alloc_r[index], &tempr);
    seg_swap(&is_buddy_alloc[index], &temprA);
    index++;
  }

  seg_buddy_alloc_count++;
}

int seg_buddy_fit(int index, int size)
{
  while (size <= seg_size_buddy(index) / 2)
  {
    seg_buddy_propagation(index);
  }
  return index;
}

int seg_buddy_extend(int index, int heap, int size)
{
  seg_buddy_update_free_list();

  int next = index + ((heap) ? 1 : -1);

  while (next < seg_buddy_alloc_count && is_buddy_alloc[next] < 0)
  {

    if (seg_size_buddy(next) >= size)
    {
      next = seg_buddy_fit(next, size);
      size -= seg_size_buddy(next);
    }

    if (size <= 0)
    {
      return next;
    }

    size -= seg_size_buddy(next);
    next = next + ((heap) ? 1 : -1);
  }

  return -1;
}

int seg_find_self_malloc(int size)
{
  int free = 0;
  addr_t li = seg_heap[seg_cur_ipid];

  fori(seg_pa(0), seg_heap[seg_cur_ipid])
  {
    free++;
    li = min(li, i);

    if (seg_virtual_memory[i] >= 0)
    {
      li = seg_heap[seg_cur_ipid];
      free = 0;
    }

    if (free >= size)
    {
      return li - seg_pa(0);
    }
  }
  return -1;
}

int seg_buddy_malloc(int size, int strategy(int *arr, int count, int size))
{
  int slots[default_size_of_arrays];
  int count = 0;

  for (int i = 0; i < seg_buddy_alloc_count; i++)
  {
    if (is_buddy_alloc[i] < 0 && size <= seg_size_buddy(i))
    {
      slots[count] = i;
      count++;
    }
  }

  if (count > 0) // existe un espacio en que el proceso pueda ser alojado
  {
    int selected = strategy(slots, count, size);
    selected = seg_buddy_fit(slots[selected], size);
    return selected;
  }

  return -1;
}
// Strategies
int worst_fit(int *arr, int count, int size)
{
  int ans = -1;
  int val = -1;
  fori(0, count)
  {
    if (val < seg_size_buddy(arr[i]))
    {
      ans = i;
      val = min(ans, arr[i]);
    }
  }
  return ans;
}

int best_fit(int *arr, int count, int size)
{
  int ans = -1;
  int val = __INT_MAX__;
  fori(0, count)
  {
    if (val > seg_size_buddy(arr[i]))
    {
      ans = i;
      val = min(ans, arr[i]);
    }
  }
  return ans;
}

int first_fit(int *arr, int count, int size)
{
  for (int i = 0; i < count; i++)
  {
    if (seg_size_buddy(arr[i]) >= size)
    {
      return i;
    }
  }
  return -1;
}

int random_fit(int *arr, int count, int size)
{
  int selected = rand() % count;
  int x = 0;

  while (size > arr[selected] && x < count)
  {
    int selected = rand() % count;
    x++;
  }
  return size <= arr[selected] ? selected : first_fit(arr, count, size);
}

int get_segment(int heap)
{
  int last = (heap) ? -1 : __INT_MAX__;

  for (int i = 0; i < seg_buddy_alloc_count; i++)
  {
    if (is_buddy_alloc[i] == seg_pids[seg_cur_ipid])
    {
      last = (heap) ? max(last, i) : min(last, i);
    }
  }
  return last;
}

int mag_log2(int x)
{
  return x > 0 ? 1 + mag_log2(x / 2) : 0;
}

// END

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
  seg_cur_ipid = -1;
  seg_pids = malloc(sizeof(int) * MAX_PROC_COUNT);
  seg_base = malloc(sizeof(addr_t) * MAX_PROC_COUNT);
  seg_stack = malloc(sizeof(size_t) * MAX_PROC_COUNT);
  seg_heap = malloc(sizeof(size_t) * MAX_PROC_COUNT);
  seg_stack_bound = malloc(sizeof(size_t) * MAX_PROC_COUNT);
  seg_offset = mag_log2(m_size()) + 1;

  fori(0, MAX_PROC_COUNT)
  {
    seg_pids[i] = -1;
    seg_base[i] = 0;
    seg_stack[i] = (1 << seg_offset) + 1;
    seg_heap[i] = 0;
    seg_stack_bound[i] = DEFAULT_STACK;
  }

  seg_virtual_memory = malloc(sizeof(int) * m_size());

  seg_buddy_alloc_l = malloc(sizeof(int) * m_size());
  seg_buddy_alloc_r = malloc(sizeof(int) * m_size());
  seg_buddy_alloc_count = 0;
  is_buddy_alloc = malloc(sizeof(int) * m_size());

  fori(0, m_size())
  {
    seg_virtual_memory[i] = -1;
    is_buddy_alloc[i] = -1;
  }

  seg_buddy_alloc_l[0] = 0;
  seg_buddy_alloc_r[0] = m_size();
  seg_buddy_alloc_count++;
}
void update_heap()
{
  seg_heap[seg_cur_ipid] = 0;
  fori(0, m_size()) if (seg_virtual_memory[i] == seg_pids[seg_cur_ipid] && i >= seg_base[seg_cur_ipid]) seg_heap[seg_cur_ipid] = i - seg_base[seg_cur_ipid] + 1;
}

int is_new()
{
  fori(0, seg_buddy_alloc_count) if (is_buddy_alloc[i] == seg_pids[seg_cur_ipid]) return 1;
  return 0;
}

void update_pages()
{
  for (int i = 0; i < seg_buddy_alloc_count; i++)
  {
    int flag = 1;

    for (int j = seg_buddy_alloc_l[i]; j < seg_buddy_alloc_r[i]; j++)
    {
      if (seg_virtual_memory[j] >= 0)
      {
        flag = 0;
        break;
      }
    }

    if (flag)
    {
      is_buddy_alloc[i] = -1;
    }
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
  int index = -1;

  update_heap();

  if (is_new()) // no es nuevo
  {
    // printf("El proceso no es nuevo\n");

    index = seg_find_self_malloc(size);

    int h_seg = get_segment(1);

    // printf("Heap:%ld Seg:%d\n", seg_heap[seg_cur_ipid], h_seg);

    if (index < 0)
    {
      int free = seg_buddy_alloc_r[h_seg] - seg_pa(seg_heap[seg_cur_ipid]);
      index = seg_buddy_extend(h_seg, 1, size - free);

      if (index < 0)
      {
        printf("No hay espacio\n");
        // print_state();
        return -1;
      }

      while (h_seg <= index)
      {
        is_buddy_alloc[h_seg] = seg_pids[seg_cur_ipid];
        h_seg++;
      }

      // printf("Size %ld  Free %d\n", size, free);
      // printf("Extend %ld %ld\n", seg_pa(seg_heap[seg_cur_ipid]), seg_pa(seg_heap[seg_cur_ipid]) + size);
      // printf("Heap addrt: %ld  pa 0x%zx\n", seg_heap[seg_cur_ipid], seg_pa(seg_heap[seg_cur_ipid]));

      seg_alloc_free_memory(seg_pa(seg_heap[seg_cur_ipid]), size, 1);

      index = seg_heap[seg_cur_ipid];
      seg_heap[seg_cur_ipid] += size;
      //print_state();
    }

    out->addr = index;
    out->size = size;
  }

  else
  {

    int total_size = size + DEFAULT_STACK;
    index = seg_buddy_malloc(total_size, first_fit); // segmento

    if (index < 0)
    {
      for (int i = 0; i < seg_buddy_alloc_count; i++)
      {
        if (is_buddy_alloc[i] < 0)
        {
          int ans = seg_buddy_extend(i, 1, total_size);

          if (ans >= 0)
          {
            index = i;

            while (i <= ans)
            {
              is_buddy_alloc[i] = seg_pids[seg_cur_ipid];
              i++;
            }
            break;
          }
        }
      }

      if (index < 0)
        return -1;
    }

    seg_base[seg_cur_ipid] = seg_buddy_alloc_l[index] + DEFAULT_STACK;
    is_buddy_alloc[index] = seg_pids[seg_cur_ipid];

    out->addr = seg_heap[seg_cur_ipid];
    out->size = size;

    // printf("Proceso nuevo\n");
    // printf("Segmento %d %d\n", seg_buddy_alloc_l[index], seg_buddy_alloc_r[index]);
    // printf("Reservando %ld  hasta %ld\n", seg_base[seg_cur_ipid] - DEFAULT_STACK, seg_base[seg_cur_ipid] + out->size);

    seg_alloc_free_memory(seg_buddy_alloc_l[index], total_size, 1);
    seg_heap[seg_cur_ipid] += size;
  }
  // print_state();
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
  update_heap();

  if (!(ptr.addr >> seg_offset) && ptr.addr + ptr.size <= seg_heap[seg_cur_ipid])
  {
    int a = seg_alloc_free_memory(seg_pa(ptr.addr), ptr.size, 0);
    update_pages();
    seg_buddy_update_free_list();
    // print_state();
    return 0;
  }
  // printf("heap %ld ptr: %ld %ld\n", seg_pa(seg_heap[seg_cur_ipid]), seg_pa(ptr.addr), seg_pa(ptr.addr + ptr.size));
  // print_state();
  return 1;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{

  if (off(seg_stack[seg_cur_ipid]) >= seg_stack_bound[seg_cur_ipid])
  {
    if (seg_buddy_extend(get_segment(0), 0, 1))
      return 1;

    seg_stack_bound[seg_cur_ipid]++;
    m_set_owner(seg_pa(seg_stack[seg_cur_ipid]), seg_pa(seg_stack[seg_cur_ipid]));
  }

  out->addr = seg_stack[seg_cur_ipid];
  out->size = 1;
  m_write(seg_pa(seg_stack[seg_cur_ipid]), val);

  seg_stack[seg_cur_ipid]++;
  return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
  if (off(seg_stack[seg_cur_ipid]) - 1 <= 0)
  {
    printf("Pop stack:%ld addr:0x%zx offset: %ld  offset-1: %ld\n", seg_stack[seg_cur_ipid], seg_stack[seg_cur_ipid], off(seg_stack[seg_cur_ipid]), off(seg_stack[seg_cur_ipid]) - 1);
    return 1;
  }

  seg_stack[seg_cur_ipid]--;
  *out = m_read(seg_pa(seg_stack[seg_cur_ipid]));
  return 0;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  if (addr >= seg_heap[seg_cur_ipid] || seg_virtual_memory[seg_pa(addr)] < 0)
    return 1;

  *out = m_read(seg_pa(addr));
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{

  if (addr >= seg_heap[seg_cur_ipid] || seg_virtual_memory[seg_pa(addr)] != seg_pids[seg_cur_ipid])
  {
    printf("Memory Store: addr:0x%zx heap:0x%zx pa:%ld Owner:%d\n", addr, seg_heap[seg_cur_ipid], seg_pa(addr), seg_virtual_memory[seg_pa(addr)]);
    return 1;
  }
  m_write(seg_pa(addr), val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
  seg_cur_ipid = seg_find_pid(process.pid);

  if (seg_cur_ipid < 0)
  {
    seg_cur_ipid = seg_free_pid();
    if (seg_cur_ipid < 0)
    {
      printf("CAMBIO DE CONTEXTO: pid: %d No hay espacios libres\n", process.pid);
    }
    seg_pids[seg_cur_ipid] = process.pid;
  }
  set_curr_owner(process.pid);
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
  int temp = seg_cur_ipid;

  seg_cur_ipid = seg_find_pid(process.pid);

  if (seg_cur_ipid < 0)
  {
    printf("FIN DE PROCESO: pid: %d No existe\n", process.pid);
    return;
  }

  // printf("FIN DE PROCESO: pid: %d\n", process.pid);
  // printf("Liberando memoria\n");

  update_heap();
  int li = seg_base[seg_cur_ipid] - seg_stack_bound[seg_cur_ipid];
  int ls = seg_pa(seg_heap[seg_cur_ipid]) - li;

  seg_alloc_free_memory(li, ls, 0);

  seg_pids[seg_cur_ipid] = -1;
  seg_stack_bound[seg_cur_ipid] = DEFAULT_STACK;
  seg_stack[seg_cur_ipid] = (1 << seg_offset) + 1;
  seg_heap[seg_cur_ipid] = 0;
  seg_base[seg_cur_ipid] = 0;

  fori(0, seg_buddy_alloc_count)
  {
    if (is_buddy_alloc[i] == seg_pids[seg_cur_ipid])
    {
      is_buddy_alloc[i] = -1;
    }
  }

  seg_buddy_update_free_list();
  // print_state();
  seg_cur_ipid = temp;
}