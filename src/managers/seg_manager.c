#include "seg_manager.h"
#include "shared.h"

#include "stdio.h"

static seg_free_list_t seg_list; 
static process_t curr_proc;

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv) {
  seg_list = (seg_free_list_t) {
    .count = 0, 
    .max_count = seg_get_segment_count(m_size()),
    .segments = malloc(seg_get_segment_count(m_size()) * sizeof(seg_segment_t))
  };

  // Setting invalid pid to identify empty segment
  for (int i = 0; i < seg_list.max_count; i++) {
    seg_list.segments[i].proc.pid = -1;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out) {
  int count = 0;

  for (int i = 0; i < seg_list.max_count; i++) {
    seg_segment_t *segment = &seg_list.segments[i];

    // Avoid empty segments
    if (segment->proc.pid == -1) continue;

    // Get to a valid heap segment for this process
    if (curr_proc.pid == segment->proc.pid && segment->increase) {
      // Check offset + size stays inside segment bounds
      if (segment->offset + size < MAX_SEGMENT_SIZE) continue;
        // Address relative to segment
        addr_t addr = allocate_segment(segment->free_list, size);

        // Global address for pointer
        out->addr = segment->base + addr;
        out->size = size;
        // Set segment offset to end of the free list
        segment->offset = free_list_end(segment->free_list);

        return MEM_SUCCESS;
    }

    count++;

    if (count == seg_list.count) break;
  }

  // If there is no space or allocated heap, add the segment
  seg_segment_t *segment = seg_allocate_segment(&seg_list, curr_proc, 1);

  if (segment == NULL) return MEM_FAIL;

  // Address relative to segment
  addr_t addr = allocate_segment(segment->free_list, size);

  // Global address for pointer
  out->addr = segment->base + addr;
  out->size = size;
  // Set segment offset to end of the free list
  segment->offset = free_list_end(segment->free_list);

  return MEM_SUCCESS;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr) {
  int seg_index = ptr.addr / MAX_SEGMENT_SIZE;
  int ptr_start = ptr.addr % MAX_SEGMENT_SIZE;

  seg_segment_t *segment = &seg_list.segments[seg_index];

  unallocate_segment(segment->free_list, ptr_start);

  segment->offset = free_list_end(segment->free_list);

  for (int i = 0; i < segment->store_count; i++) {
    if (segment->store[i].address == ptr.addr) {
      segment->store_count--;

      for (int j = i; j < segment->store_count; j++) {
        segment->store[j] = segment->store[j + 1];
      }
    }
  }

  return MEM_SUCCESS;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out) {
  int count = 0;
  
  for (int i = 0; i < seg_list.max_count; i++) {
    seg_segment_t *segment = &seg_list.segments[i];

    // Avoid empty segments
    if (segment == NULL || segment->proc.pid == -1) continue;

    // Get to a valid heap segment for this process
    if (curr_proc.pid == segment->proc.pid && !segment->increase) {
      // Check offset + size stays inside segment bounds
      if (segment->offset + sizeof(byte) > MAX_SEGMENT_SIZE) continue;
        // Set segment offset
      segment->offset += sizeof(byte);

      out->addr = segment->base - segment->offset;
      out->size = sizeof(byte);

      if (segment->store == NULL || segment->store_count == 0) {
        segment->store = malloc(sizeof(store_t));
      } else {
        void *status = realloc(segment->store, (segment->store_count + 1) * sizeof(store_t));

        if (status == NULL) return MEM_FAIL;
      }

      store_t store;
      store.address = segment->base - segment->offset;
      store.value = val;

      segment->store[segment->store_count] = store;
      segment->store_count++;

      return MEM_SUCCESS;
    }

    count++;

    if (count == seg_list.count) break;
  }

  // If there is no space or allocated stack, add the segment
  seg_segment_t *segment = seg_allocate_segment(&seg_list, curr_proc, 0);

  if (segment == NULL) return MEM_FAIL;

  // Set segment offset
  segment->offset += sizeof(byte);
  // Global address for pointer
  out->addr = segment->base - segment->offset;
  out->size = sizeof(byte);

  if (segment->store == NULL || segment->store_count == 0) {
    segment->store = malloc(sizeof(store_t));
  } else {
    void *status = realloc(segment->store, (segment->store_count + 1) * sizeof(store_t));

    if (status == NULL) return MEM_FAIL;
  }

  store_t store;
  store.address = segment->base - segment->offset;
  store.value = val;

  segment->store[segment->store_count] = store;
  segment->store_count++;


  return MEM_SUCCESS;
}

// Quita un elemento del stack
int m_seg_pop(byte *out) {
  int count = 0;

  // fix pop order
  for (int i = 0; i < seg_list.max_count; i++) {
    seg_segment_t *segment = &seg_list.segments[i];

    // Avoid empty segments
    if (segment == NULL || segment->proc.pid == -1) continue;

    // Get to a valid heap segment for this process
    if (curr_proc.pid == segment->proc.pid && !segment->increase) {
        // Set segment offset
        segment->offset -= sizeof(byte);

        *out = segment->store[segment->store_count - 1].value;

        // Avoid reallocating 0 bytes
        if (segment->store_count > 1) {
          void *status = realloc(segment->store, segment->store_count - 1);
          if (status == NULL) return MEM_FAIL;
        }

        segment->store_count--;
        return MEM_SUCCESS;
    }

    count++;

    if (count == seg_list.count) break;
  }

  return MEM_FAIL;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out) {
  int seg_index = addr / MAX_SEGMENT_SIZE;

  seg_segment_t *segment = &seg_list.segments[seg_index];

  for (int i = 0; i < segment->store_count; i++) {
    if (segment->store[i].address == addr) {
      *out = segment->store[i].value;
      return MEM_SUCCESS;
    }
  }

  return MEM_FAIL;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val) {
  int seg_index = addr / MAX_SEGMENT_SIZE;
  
  seg_segment_t *segment = &seg_list.segments[seg_index];

  if (segment->store == NULL || segment->store_count == 0) {
    segment->store = malloc(sizeof(store_t));
  } else {
    void *status = realloc(segment->store, (segment->store_count + 1) * sizeof(store_t));

    if (status == NULL) return MEM_FAIL;
  }

  segment->store[segment->store_count] = (store_t) {.address = addr, .value = val};
  segment->store_count++;

  return MEM_SUCCESS;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process) {
  curr_proc = process;
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process) {
  int count = 0;

  for (int i = 0; i < seg_list.max_count; i++) {
    seg_segment_t *segment = &seg_list.segments[i];

    // Avoid empty segments
    if (segment == NULL) continue;

    if (segment->proc.pid == process.pid) {
      segment->proc.pid = -1;
      seg_list.count--;
    }

    count++;

    if (count == seg_list.count) break;
  }
}
