#ifndef MEM_SIM_H
#define MEM_SIM_H

#include "manager.h"
#include "memory.h"
#include "utils.h"

void setup_sim(size_t mem_size, const char *log_file, int argc, char **argv);

void mem_store(addr_t addr, byte val);
void mem_store_assert(addr_t addr, byte val, int result);
byte mem_load(addr_t addr);
byte mem_load_assert(addr_t addr, byte val);
ptr_t mem_malloc(size_t size);
void mem_free(ptr_t ptr);
ptr_t mem_push(byte val);
byte mem_pop();
byte mem_pop_assert(byte val);

void ctx_switch(process_t process);
void end_process(process_t process);
void assert_eq(byte val_1, byte val_2);
void end_sim();

#define at(ptr) ptr.addr
#define pstore(ptr, val) store(at(ptr), val)
#define pload(ptr) load(at(ptr))

#endif  // !MEMORY_H
