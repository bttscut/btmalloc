#include "btmalloc.h"
#include "stdio.h"
#include <stdlib.h>
#include <time.h>

int main() {
    if (mm_init() != 0) {
        fprintf(stderr, "mm_init failed\n");
        return 1;
    }
    char* l[100];
    int total = 0;
    printf("=============\n");
    srand(time((time_t*)NULL));
    for(int i=0;i<500;i++) {
        int rd = rand()%10000+1;
        total += rd;
        char* p = mm_malloc(rd);
        mm_stat();
        l[i] = p;
    }
    for(int i=0;i<100;i++) {
        mm_free(l[i]);
        mm_stat();
    }
    printf("total: %d\n", total);
    return 0;
}
