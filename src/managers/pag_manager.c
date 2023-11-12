#include "pag_manager.h"
#include "../utils.h"

#include "math.h"
#include "stdio.h"

typedef struct StackStruct {
  int stack_size;
  size_t *stack_s;
  int top;
} stack_struct;
typedef struct ProcessStruct {
  int pid; 
  size_t process_length;
  size_t *process;
  size_t **process_free;
  stack_struct *stack;
} process_struct;
typedef struct ListStruct {
  size_t length;
  size_t size;
  process_struct *data;
} process_list_struct;

stack_struct *init_stack(int size) {
  stack_struct *stack_init = (stack_struct *)malloc(sizeof(stack_struct));
  stack_init->stack_s = (size_t *)malloc(size * sizeof(size_t));
  stack_init->stack_size = size;
  stack_init->top = -1;
  return stack_init;
}
void free_stack(stack_struct *stack) {
  free(stack->stack_s);
  free(stack);
}
void push_stack(stack_struct *stack, size_t addr) {
  if (stack->top == stack->stack_size - 1) return 1;
  stack->top++;
  stack->stack_s[stack->top] = addr;
}
size_t pop_stack(stack_struct *stack) {
  if (stack->top == -1) return 0;
  size_t value = stack->stack_s[stack->top];
  stack->top--;
  return value;
}

process_struct *init_process(int pid, size_t size, size_t table_frame_len) {
  process_struct *process_init = (process_struct *)malloc(sizeof(process_struct));
  process_init->pid = pid;
  process_init->process = (size_t **)malloc(table_frame_len * sizeof(size_t *));
  process_init->process_length = 0;
  process_init->process_free = (size_t **)malloc(table_frame_len * sizeof(size_t *));
  for (size_t i = 0; i < table_frame_len; i++) {
    process_init->process_free[i] = (size_t *)malloc(size * sizeof(size_t));
  }
  process_init->stack = init_stack(table_frame_len);
  return process_init;
}
void free_process(process_struct *process_free) {
  free_stack(process_free->stack);
  free(process_free->process_free);
  free(process_free->process);
  free(process_free);
}

process_list_struct *init_list() {
  process_list_struct *list = (process_list_struct *)malloc(sizeof(process_list_struct));
  list->length = 0;
  list->size = 5;
  list->data = (process_struct *)malloc(list->size * sizeof(process_struct));
  return list;
}
void push_list(process_list_struct *list, process_struct x) {
  if (list->size == list->length + 1) {
    list->size = list->size*2;
    list->data = (process_struct *)realloc(list->data, list->size * sizeof(process_struct)); 
  }
  list->data[list->length] = x;
  list->length = list->length + 1;
}
int remove_position_list(process_list_struct *list, size_t position) {
  if (list->length <= position) return -1;
  for (size_t i = position; i < list->length; i++) {
    list->data[i] = list->data[i+1];
  }
  list->length = list->length - 1;
  return 1;
}
int contains_list(process_list_struct *list, int pid) {
  for (size_t i = 0; i < list->length; i++) {
    process_struct process_temp = list->data[i];
    if (process_temp.pid == pid) return i;
  } return -1;
}

static size_t *table_frame;
static size_t table_frame_length;
static const size_t page_size = 128;
static const size_t page_frame_count = 7; 
static process_list_struct *process_list;
static process_struct *process_actual;

// PAGINACION
void m_pag_init(int argc, char **argv) {
  size_t memory = m_size();
  table_frame_length = memory/page_size;
  table_frame = (size_t *)malloc(table_frame_length * sizeof(size_t));
  process_list = init_list();
}
int m_pag_free(ptr_t ptr) {
  if (ptr.size > page_size) {
    size_t addr = ptr.size;
    size_t pag = ptr.addr >> page_frame_count; 
    size_t position = ptr.addr - (page_size * pag);
    while (1) {
      table_frame[process_actual->process[pag]] = 0;
      m_unset_owner(process_actual->process[pag]*page_size, (process_actual->process[pag] + 1) * page_size);
      process_actual->process[pag] = 0;
      if (addr >= page_size) {
        addr -= page_size;
        for (size_t i = 0; i < page_size; i++) {
          process_actual->process_free[pag][i] = 0;
        }
      } else {
        for (size_t i = 0; i < addr; i++) {
          process_actual->process_free[pag][i] = 0;
        }
        addr = 0;
      }
      pag += 1;
      if (addr == 0) return 0;
    } 
  } else {
    size_t pag = ptr.addr >> page_frame_count;
    size_t position = ptr.addr - (page_size*pag); 
    for (size_t i = 0; i < ptr.size; i++) {
      if (position >= page_size) {
        position = 0;
        pag++;
      } 
      size_t addr_physical = (process_actual->process[pag] * page_size) + position + i;
      process_actual->process_free[pag][position + i] = 0;
      m_write(addr_physical, 0);
    } return 0;
  }
}
int m_pag_malloc(size_t size, ptr_t *out) {
  if (size > page_size) {
    size_t count = size; 
    process_actual->process_length++;
    size_t pag = size >> page_frame_count; 
    out->addr = (process_actual->process_length - 1)*page_size;
    out->size = size;
    for (size_t i = 0; i < table_frame_length; i++) {
      if (table_frame[i] != 1) {
        table_frame[i] = 1;
        process_actual->process[process_actual->process_length - 1] = i;
        m_set_owner(i*page_size, (i+1)*page_size);
        if (count >= page_size) {
          count -= page_size;
          for (size_t k = 0; k < page_size; k++) {
            process_actual->process_free[process_actual->process_length - 1][k] = 1;
          } 
          process_actual->process_length++;
        } else {
          for (size_t k = 0; k < count; k++) {
            process_actual->process_free[process_actual->process_length - 1][k] = 1;
          } 
          count = 0;
        } 
        if (count == 0) return 0;
      }
    }
    return 1;
  } else {
    int condition = 1;
    for (size_t i = 0; i < process_actual->process_length; i++) {
      for (size_t j = 0; j < page_size; j++) {
        if (process_actual->process_free[i][j] != 1) {
          size_t k = 0;
          for (; k < size && ((j + k) < page_size); k++) {
            if (process_actual->process_free[i][j + k] == 1) break;
          }
          if (k == size) {
            out->addr = (i*page_size) + j;
            out->size = size;
            for (size_t k = 0; k < size; k++) {
              process_actual->process_free[i][j + k] = 1;
            }
            condition = 0;
            return condition;
          } else j+=k;
        } 
      }
    }
    if (condition) {
      for (size_t i = 0; i < table_frame_length; i++) {
        if (table_frame[i] != 1) {
          table_frame[i] = 1;
          process_actual->process_length++;
          process_actual->process[process_actual->process_length-1] = i;
          if (process_actual->process_length > table_frame_length) return 1;
          size_t addr_physical = (process_actual->process[process_actual->process_length - 1]*page_size);
          m_set_owner(addr_physical,addr_physical+page_size);
          out->size = size;
          out->addr = (process_actual->process_length - 1)*page_size;
          for (size_t j = 0; j < size; j++){
            process_actual->process_free[process_actual->process_length - 1][j] = 1;
          }
          condition = 0;
          return condition;
        }
      }
    }
    return condition;
  }
}
int m_pag_push(byte val, ptr_t *out) {
  for (size_t i = 0; i < process_actual->process_length; i++) {
    for (size_t j = 0; j < page_size; j++) {
      if (process_actual->process_free[i][j] != 1) {
        process_actual->process_free[i][j] = 1;
        size_t addr_physical = (process_actual->process[i]*page_size) + j;
        out->addr = (i*page_size) + j;
        out->size = 1;
        m_write(addr_physical, val);
        push_stack(process_actual->stack, out->addr);
        return 0;
      }
    }
  }
  process_actual->process_length++; 
  if (process_actual->process_length > table_frame_length) return 1;
  process_actual->process_free[process_actual->process_length - 1][0] = 1;
  size_t addr_physical =(process_actual->process[process_actual->process_length-1]*page_size);
  out->addr = (process_actual->process_length - 1)*page_size;
  out->size = 1;
  m_write(out->addr, val);
  push_stack(process_actual->stack, out->addr);
  return 0;
}
int m_pag_pop(byte *out) {
  size_t addr = pop_stack(process_actual->stack);
  size_t pag = addr >> page_frame_count;
  *out = m_read((process_actual->process[pag])*page_size+(addr-(page_size*pag)));
  process_actual->process_free[pag]--;
  return 0;
}
int m_pag_load(addr_t addr, byte *out) {
  size_t pag = addr >> page_frame_count;
  size_t position = addr - (page_size * pag);
  size_t addr_physical = (process_actual->process[pag] * page_size) + position;
  *out = m_read(addr_physical);
  return 0;
}
int m_pag_store(addr_t addr, byte val) {
  size_t pag = addr >> page_frame_count; 
  size_t position = addr - (page_size * pag); 
  size_t addr_physical = (process_actual->process[pag] * page_size) + position;
  m_write(addr_physical, val);
  return 0;
}
void m_pag_on_ctx_switch(process_t process) {
  int position = contains_list(process_list, process.pid);
  if (position != -1) {
    *process_actual = process_list->data[position];
    set_curr_owner(process.pid);
  } else {
    size_t size_process = process.program->size;
    size_t code_count = process.program->size / page_size;
    if (process.program->size % page_size != 0) code_count++;
    if (process.program->size == 0) code_count = 1;
    process_actual = init_process(process.pid, page_size, table_frame_length);
    set_curr_owner(process.pid);
    size_t j = 0;
    for (size_t i = 0; i < table_frame_length; i++) {
      if (table_frame[i] != 1) {
        m_set_owner(i * page_size, (i+1)*page_size);
        table_frame[i] = 1;
        if (size_process >= page_size) {
          size_process -= page_size;
          for (size_t k = 0; k < page_size; k++) process_actual->process_free[j][k] = 1;
        } else {
          for (size_t k = 0; k < size_process; k++) process_actual->process_free[j][k] = 1;
        }
        process_actual->process[j] = i;
        process_actual->process_length++;
        j++;
        if (j == code_count) break;
      }
    } 
    if (j != code_count) exit(1);
    push_list(process_list, *process_actual);
  }
  
}
void m_pag_on_end_process(process_t process) {
  set_curr_owner(process.pid);
  int position = contains_list(process_list, process.pid);
  if (position != -1) {
    process_struct *process_temp = &process_list->data[position];
    for (size_t i = 0; i < process_temp->process_length; i++) {
      table_frame[process_temp->process[i]] = 0;
      m_unset_owner(process_temp->process[i] * page_size, (process_temp->process[i] + 1) * page_size);
    }
    free_process(process_temp);
    remove_position_list(process_list, position);
  }
}
