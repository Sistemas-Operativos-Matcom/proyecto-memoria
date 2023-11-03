#include <stdlib.h>
#include <stdbool.h>
#include "pag_process.h"

void pag_process_init(pag_process_t* proc, int pid, size_t size) {
    proc->pid = pid;
    proc->vpn = (size_t*) malloc(size * sizeof(size_t));
    proc->valid = (bool*) malloc(size * sizeof(bool));
    proc->total_mem = size;
    proc->is_active = true;
    proc->stack_point = 0xffffffffffffffff;

    for (size_t i = 0; i < size; i++) {
        proc->valid[i] = false;
    }
}


size_t find_space(const pag_process_t* proc, size_t size) {
    size_t count = 0;
    size_t last_pos = 0;
    
    for (size_t i = 0; i < proc->total_mem; i++) {
        if (count == size)return last_pos;
        if (!proc->valid[i])count++;
        else {
            count = 0;
            last_pos = i + 1;
        }
    }

    return ~0;
}

size_t _log2(size_t n) {
    size_t ans = -1;
    
    while (n) {
        ans ++;
        n >>= 1;
    }
    
    return ans;
}