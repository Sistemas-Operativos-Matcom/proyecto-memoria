#include "memory.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static memory_t g_mem = NULL;
static int *g_ownership = NULL;
static int g_curr_owner = NO_ONWER;
static size_t g_mem_size;
static FILE *g_log_file;
static size_t g_free_mem;

static void clean() {
  if (g_log_file != NULL) {
    fclose(g_log_file);
    g_log_file = NULL;
  }
  if (g_mem != NULL) {
    free(g_mem);
    g_mem = NULL;
  }
  if (g_ownership != NULL) {
    free(g_ownership);
    g_ownership = NULL;
  }
}

static void error_exit() {
  clean();
  exit(1);
}

void mem_init(size_t size, const char *log_file_path) {
  clean();

  g_mem_size = size;
  g_free_mem = size;
  g_mem = (memory_t)malloc(g_mem_size);
  g_ownership = (int *)malloc(g_mem_size * sizeof(int));
  for (size_t i = 0; i < g_mem_size; i++) {
    g_ownership[i] = NO_ONWER;
  }

  g_log_file = fopen(log_file_path, "w+");
  assert(g_log_file);
  fprintf(g_log_file, "%lu\n", size);
}

size_t m_size() { return g_mem_size; }

static int valid_addr(addr_t addr) { return addr < g_mem_size; }

void m_write(addr_t addr, byte val) {
  if (!valid_addr(addr)) {
    fprintf(stderr,
            "[ERROR]: Invalid address 0x%zx when writing in physical memory\n",
            addr);
    error_exit();
  }
  if (g_curr_owner != g_ownership[addr]) {
    fprintf(
        stderr,
        "[ERROR]: Invalid owner %d when writing in physical memory at 0x%zx. "
        "Ownership in memory was %d.\n",
        g_curr_owner, addr, g_ownership[addr]);
    error_exit();
  }
  g_mem[addr] = val;
  fprintf(g_log_file, "w %lu\n", addr);
}

byte m_read(addr_t addr) {
  if (!valid_addr(addr)) {
    fprintf(
        stderr,
        "[ERROR]: Invalid address 0x%zx when reading from physical memory\n",
        addr);
    error_exit();
  }
  if (g_curr_owner != g_ownership[addr]) {
    fprintf(
        stderr,
        "[ERROR]: Invalid owner %d when reading from physical memory at 0x%zx. "
        "Ownership in memory was %d.\n",
        g_curr_owner, addr, g_ownership[addr]);
    error_exit();
  }
  fprintf(g_log_file, "r %lu\n", addr);
  return g_mem[addr];
}

static void validate_ownership(int owner, size_t from_addr, size_t to_addr) {
  if (!valid_addr(from_addr)) {
    fprintf(stderr,
            "[ERROR] Ownership starting address must be lower than 0x%zx\n",
            g_mem_size);
    error_exit();
  }
  if (!valid_addr(to_addr)) {
    fprintf(stderr,
            "[ERROR] Ownership ending address must be lower than 0x%zx\n",
            g_mem_size);
    error_exit();
  }
  if (to_addr < from_addr) {
    fprintf(stderr,
            "[ERROR] Ownership ending address (0x%zx) must be grater than "
            "starting address (0x%zx)\n",
            to_addr, from_addr);
    error_exit();
  }
  if (owner < 0 && owner != NO_ONWER) {
    fprintf(stderr, "[ERROR] Invalid owner %d\n", owner);
    error_exit();
  }
}
void m_unset_owner(size_t from_addr, size_t to_addr) {
  validate_ownership(NO_ONWER, from_addr, to_addr);
  for (size_t i = from_addr; i <= to_addr; i++) {
    g_ownership[i] = NO_ONWER;
    g_free_mem++;
  }
  fprintf(g_log_file, "o %d %lu %lu %lu\n", NO_ONWER, from_addr, to_addr,
          g_free_mem);
}

void m_set_owner(size_t from_addr, size_t to_addr) {
  validate_ownership(g_curr_owner, from_addr, to_addr);
  for (size_t i = from_addr; i <= to_addr; i++) {
    if (g_ownership[i] == NO_ONWER) {
      g_free_mem--;
    }
    g_ownership[i] = g_curr_owner;
  }
  fprintf(g_log_file, "o %d %lu %lu %lu\n", g_curr_owner, from_addr, to_addr,
          g_free_mem);
  printf("[INFO] Setting ownership %d from addr 0x%zx to addr 0x%zx\n",
         g_curr_owner, from_addr, to_addr);
}

void set_curr_owner(int owner) {
  assert(owner >= 0 || owner == NO_ONWER);
  g_curr_owner = owner;
}

void mem_end() {
  fprintf(g_log_file, "end");
  clean();
}
