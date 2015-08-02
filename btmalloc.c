#include "btmalloc.h"
#include "memlib.h"
#include <stdlib.h>
#include <stdio.h>

#define WSIZE 4
#define DSIZE 8
#define MIN_BLOCK_SIZE (2*DSIZE)
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ( (x) > (y)? (x):(y) )

#define PACK(size, alloc) ( (size) | (alloc) )

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val)) 

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))


void *heap_listp;

static void *coalesce(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    if (prev_alloc && next_alloc) {
        return bp;
    }
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void* extend_heap(size_t words) {
    char * bp;
    size_t size;

    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    
    return coalesce(bp);
}

static void* find_fit(size_t size) {
    void *it = heap_listp + DSIZE;
    while (!(GET_SIZE(HDRP(it)) == 0 && GET_ALLOC(HDRP(it)))) {
        if (GET_ALLOC(HDRP(it)) == 0 && GET_SIZE(HDRP(it)) > size)
            return it;
        it = NEXT_BLKP(it);
    }
    return NULL;
}

static void place(void *bp, size_t size) {
    size_t total = GET_SIZE(HDRP(bp));
    size_t remain = total - size;

    if (remain >= MIN_BLOCK_SIZE) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remain, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remain, 0));
    }
}


int mm_init(void) {
    mem_init();
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}    

void mm_free(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

void* mm_malloc(size_t size) {
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
        return NULL;

    if (size < DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + DSIZE +(DSIZE - 1)) / DSIZE);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

void mm_stat() {
    void *it = heap_listp + DSIZE;
    int counter = 0;
    printf("----------------------\n");
    while (!(GET_SIZE(HDRP(it)) == 0 && GET_ALLOC(HDRP(it)))) {
        printf("[%d]: size:<%zu> al:<%d>\n", counter, GET_SIZE(HDRP(it)), GET_ALLOC(HDRP(it)));
        counter += 1;
        it = NEXT_BLKP(it);
        if (counter > 4) {
            break;
        }
    }
    printf("----------------------\n");
}
