#include <stdio.h>
#include <stdlib.h>

#include "space_block.h"

void validate_bounds(addr_t start, addr_t end)
{
    if (start < 0)
        fprintf(stderr, "Arguments must be non negative"), exit(1);
    if (start >= end)
        fprintf(stderr, "\"start\" must be strictly smaller than \"end\""), exit(1);
}

space_block new_space_block(addr_t start, addr_t end)
{
    space_block _space_block = (space_block)malloc(sizeof(struct space_block));

    validate_bounds(start, end);

    _space_block->start = start;
    _space_block->end = end;
    _space_block->size = end - start;
    return _space_block;
}