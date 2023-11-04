#include "mem_sim.h"
#include <stdio.h>
#include "manager.h"
#include "memory.h"
#include "utils.h"

static int g_cpid = -1;

void setup_sim(size_t mem_size, const char *log_file, int argc, char **argv) {
  mem_init(mem_size, log_file);
  m_init(argc, argv);
  printf("Starting simulation. Memory size: %zu bytes.\n", mem_size);
}

void mem_store(addr_t addr, byte val) {
  if (m_store(addr, val)) {
    printf("[ERROR] (pid: %d) Storing %d at 0x%zx failed\n", g_cpid, val, addr);
    exit(1);
  }
  printf("[INFO] (pid: %d) Stored %d at 0x%zx\n", g_cpid, val, addr);
}

void mem_store_assert(addr_t addr, byte val, int result) {
  int m_result = m_store(addr, val);
  if (m_result != result) {
    char *m_result_str = m_result ? "succeed" : "failed";
    char *result_str = result ? "succeed" : "fail";
    printf("[ERROR] (pid: %d) Storing %d at 0x%zx %s but was expected to %s\n",
           g_cpid, val, addr, m_result_str, result_str);
    exit(1);
  }
  if (m_result == MEM_SUCCESS) {
    printf("[INFO] (pid: %d) Stored %d at 0x%zx\n", g_cpid, val, addr);
  }
}

byte mem_load(addr_t addr) {
  byte out;
  if (m_load(addr, &out)) {
    printf("[ERROR] (pid: %d) Loading at 0x%zx failed\n", g_cpid, addr);
    exit(1);
  }
  printf("[INFO] (pid: %d) Loaded %d at 0x%zx\n", g_cpid, out, addr);
  return out;
}

byte mem_load_assert(addr_t addr, byte val) {
  byte loaded_val = mem_load(addr);
  if (loaded_val != val) {
    printf(
        "[ERROR] (pid: %d) Incorrect value %d loaded at 0x%zx (should have "
        "been %d)\n",
        g_cpid, loaded_val, addr, val);
    exit(1);
  }
  return loaded_val;
}

ptr_t mem_malloc(size_t size) {
  ptr_t ptr;
  if (m_malloc(size, &ptr)) {
    printf("[ERROR] (pid: %d) Allocating %zu bytes\n", g_cpid, size);
    exit(1);
  }
  printf("[INFO] (pid: %d) Allocate %zu bytes. Result ptr: 0x%zx\n", g_cpid,
         size, at(ptr));
  return ptr;
}

void mem_free(ptr_t ptr) {
  if (m_free(ptr)) {
    printf("[ERROR] (pid: %d) Call free at ptr 0x%zx\n", g_cpid, at(ptr));
    exit(1);
  }
  printf("[INFO] (pid: %d) Called free at 0x%zx\n", g_cpid, at(ptr));
}

ptr_t mem_push(byte val) {
  ptr_t ptr;
  if (m_push(val, &ptr)) {
    printf("[ERROR] (pid: %d) Pushing %d into stack\n", g_cpid, val);
    exit(1);
  }
  printf("[INFO] (pid: %d) Pushed %d into stack. Result ptr: 0x%zx\n", g_cpid,
         val, at(ptr));
  return ptr;
}

byte mem_pop() {
  byte out;
  if (m_pop(&out)) {
    printf("[ERROR] (pid: %d) Poping from stack\n", g_cpid);
    exit(1);
  }
  printf("[INFO] (pid: %d) Poped %d from stack\n", g_cpid, out);
  return out;
}

byte mem_pop_assert(byte val) {
  byte loaded_val = mem_pop();
  if (loaded_val != val) {
    printf(
        "[ERROR] (pid: %d) Incorrect value %d popped from stack (should have "
        "been %d)\n",
        g_cpid, loaded_val, val);
    exit(1);
  }
  return loaded_val;
}

void ctx_switch(process_t process) {
  set_curr_owner(process.pid);
  g_cpid = process.pid;
  m_on_ctx_switch(process);
}

void end_process(process_t process) { m_on_end_process(process); }

void end_sim() {
  mem_end();
  printf("Finish simulation\n\n");
}
