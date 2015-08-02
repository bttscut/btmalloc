libbtmalloc.so: btmalloc.c btmalloc.h memlib.c memlib.h
	gcc -g btmalloc.c memlib.c -o $@ -shared -fPIC

test: libbtmalloc.so test.c
	gcc -std=c99 -g test.c -o test -l :./libbtmalloc.so


