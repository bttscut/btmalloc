#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

static char *mem_heap;
static char *mem_brk;
static char *mem_max_addr;

static const int MAX_HEAP = 10*1024*1024;

void mem_init(void) {
    mem_heap = (char*)malloc(MAX_HEAP);
    mem_brk = (char*)mem_heap;
    mem_max_addr = (char*)(mem_heap + MAX_HEAP);
}

void* mem_sbrk(int incr) {
    char *old_brk = mem_brk;
    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed, Run out of memory...\n");
        return (void*) -1;
    }
    mem_brk += incr;
    return (void*)old_brk;
}

