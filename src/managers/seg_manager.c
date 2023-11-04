#include <stdio.h>
#include "seg_manager.h"
#include "structs.h"
#include "memory.h"

static process_t actual_proc;

static register_list reg_list;

static lf_list frees;

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
  reg_list = Init_Register_List();
  frees = Init_LF(m_size());
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  out->addr = reg->heap_pointer;
  out->size = size;
  reg->heap_pointer += size;
  return reg->heap_pointer >= reg->heap_segment.base + reg->heap_segment.bound;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  size_t from_addr = Search_addr(ptr.addr, &reg->mask);
  size_t to_addr = from_addr + ptr.size;
  for (size_t i = to_addr; i < reg->heap_pointer; i++)
  {
    m_write(from_addr, m_read(to_addr));
    from_addr++;
  }
  for (size_t i = 0; i < reg->mask.length; i++)
  {
    if (reg->mask.start[i].addr_v >= ptr.addr)
    {
      reg->mask.start[i].addr_r -= ptr.size;
    }
  }
  reg->heap_pointer -= ptr.size;
  return 0;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  reg->stack_pointer--;
  m_write(reg->stack_pointer, val);
  return reg->stack_pointer <= reg->stack_segment.base;
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  *out = m_read(reg->stack_pointer);
  reg->stack_pointer++;
  return 0;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  size_t address = Search_addr(addr, &reg->mask);
  *out = m_read(address);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
  seg_register *reg = Search_Register(actual_proc, &reg_list);
  Add_Mask(addr, reg->heap_pointer, &reg->mask);
  m_write(reg->heap_pointer, val);
  reg->heap_pointer++;
  return reg->heap_pointer > reg->heap_segment.base + reg->heap_segment.bound;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
  if (!Exist_Register(process, &reg_list))
  {
    lf_element code_seg = Fill_Space(process.program->size, &frees);
    lf_element heap_seg = Fill_Space(128, &frees);
    lf_element stack_seg = Fill_Space(128, &frees);
    set_curr_owner(process.pid);

    printf("de %lu a %lu\n", code_seg.start, code_seg.start + code_seg.size - 1);
    printf("de %lu a %lu\n", heap_seg.start, heap_seg.start + heap_seg.size - 1);
    printf("de %lu a %lu\n", stack_seg.start, stack_seg.start + stack_seg.size - 1);

    m_set_owner(code_seg.start, code_seg.start + code_seg.size - 1);
    m_set_owner(heap_seg.start, heap_seg.start + heap_seg.size - 1);
    m_set_owner(stack_seg.start, stack_seg.start + stack_seg.size - 1);

    Add_Register(process, (segment){code_seg.start, code_seg.size}, (segment){heap_seg.start, heap_seg.size}, (segment){stack_seg.start, stack_seg.size}, &reg_list);
  }
  actual_proc = process;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
  seg_register *reg = Search_Register(process, &reg_list);
  Free_Space(reg->code_segment.base, reg->code_segment.bound, &frees);
  Free_Space(reg->heap_segment.base, reg->heap_segment.bound, &frees);
  Free_Space(reg->stack_segment.base, reg->stack_segment.bound, &frees);
  Remove_Register(process, &reg_list);
  m_unset_owner(reg->code_segment.base, reg->code_segment.base + reg->code_segment.bound - 1);
  m_unset_owner(reg->heap_segment.base, reg->heap_segment.base + reg->heap_segment.bound - 1);
  m_unset_owner(reg->stack_segment.base, reg->stack_segment.base + reg->stack_segment.bound - 1);
}
