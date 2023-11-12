#include "shared.h"

free_list_t *init_free_list() {
    free_list_t *free_list = malloc(sizeof(free_list_t));
    
    free_list->count = 0;
    free_list->segments = malloc(INITIAL_FREE_LIST_SIZE * sizeof(mem_segment_t));

    return free_list;
}

size_t mem_segment_distance(mem_segment_t m1, mem_segment_t m2) {
    return m2.start - m1.end;
}

size_t free_list_end(free_list_t *free_list) {
    return free_list->segments[free_list->count + 1].end - free_list->segments[0].start;
}

size_t get_free_segment_start(free_list_t *free_list, size_t size) {
    if (free_list ->count == 0) return 0;

    for (int i = 0; i < free_list->count; i++) {
        size_t distance = mem_segment_distance(free_list->segments[i], free_list->segments[i+1]);

        if (distance > size) {
            return free_list->segments[i].end + 1;
        }
    }

    return free_list->segments[free_list->count - 1].end + 1;
}

addr_t allocate_segment(free_list_t *free_list, size_t size) {
    int allocated = 0;
    addr_t addr;

    if (free_list->count == 0) {
        mem_segment_t segment = {.start = 0, .end = size};

        free_list->segments[0] = segment;
        free_list->count++;

        allocated = 1;
        addr = segment.start;
    } else if (free_list->count == 1) {
        mem_segment_t segment = {.start = free_list->segments[0].end + 1, .end = size};

        free_list->segments[1] = segment;
        free_list->count++;

        allocated = 1;
        addr = segment.start;
    } else {
        for (int i = 0; i < free_list->count - 1; i++) {
            size_t distance = mem_segment_distance(free_list->segments[i], free_list->segments[i+1]);

            if (distance > 0) {
                if (distance < size) {
                    mem_segment_t segment = {.start = free_list->segments[i].end + 1, .end = size};

                    free_list->segments[free_list->count] = segment;

                    for (int j = free_list->count; j > i; j++) {
                        mem_segment_t temp = free_list->segments[j];
                        free_list->segments[j] = free_list->segments[j - 1];
                        free_list->segments[j - 1] = temp;
                    }

                    free_list->count++;

                    allocated = 1;
                    addr = segment.start;

                    break;
                }
            }
        }
    }

    if (allocated) {
        return addr;
    }

    mem_segment_t segment = {.start = free_list->segments[free_list->count].end + 1, .end = size};

    free_list->segments[free_list->count] = segment;
    free_list->count++;

    return segment.start;
}

void unallocate_segment(free_list_t *free_list, addr_t start) {
    for (int i = 0; i < free_list->count; i++) {
        if (free_list->segments[i].start == start) {
            for (int j = i; j < free_list->count; j++) {
                free_list->segments[j] = free_list->segments[j + 1];
            }

            free_list->count--;
            return;
        }
    }
}

int seg_get_segment_count(size_t memory) {
    return memory / MAX_SEGMENT_SIZE;
}

seg_segment_t *seg_allocate_segment(seg_free_list_t *free_list, process_t proc, int increase) {
    seg_segment_t segment;

    if (free_list->count == 0) {
        if (increase) {
            segment = (seg_segment_t) {
                .base = 0,
                .proc = proc,
                .increase = increase,
                .offset = 0,
                .free_list = init_free_list(),
                .store_count = 0
            };
        } else {
            segment = (seg_segment_t) {
                .base = MAX_SEGMENT_SIZE,
                .proc = proc,
                .increase = increase,
                .offset = 0,
                .free_list = init_free_list(),
                .store_count = 0
            };
        }

        free_list->segments[0] = segment;
        free_list->count++;

        return &free_list->segments[0];
    } else {
        for (int i = 0; i < free_list->max_count; i++) {
            if (free_list->segments[i].proc.pid == -1) {
                if (increase) {
                    segment = (seg_segment_t) {
                        .base = i * MAX_SEGMENT_SIZE,
                        .proc = proc,
                        .increase = increase,
                        .offset = 0,
                        .free_list = init_free_list(),
                        .store_count = 0
                    };
                } else {
                    segment = (seg_segment_t) {
                        .base = (i + 1) * MAX_SEGMENT_SIZE,
                        .proc = proc,
                        .increase = increase,
                        .offset = 0,
                        .free_list = init_free_list(),
                        .store_count = 0
                    };
                }
                
                free_list->segments[i] = segment;
                free_list->count++;

                return &free_list->segments[i];
            }
        }
    }

    return NULL;
}

void seg_unallocate_segment(seg_free_list_t *free_list, int pid) {
    int count = 0;
    for (int i = 0; i < free_list->max_count; i++) {
        if (free_list->segments[i].proc.pid == pid) {
            free_list->segments = NULL;
        }
        count++;

        if (count == free_list->count) return;
    }
}
