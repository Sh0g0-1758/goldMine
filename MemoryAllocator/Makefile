build:
	gcc -o memalloc.so -fPIC -shared memAlloc.c

test: build
	gcc test.c -o test
	LD_PRELOAD=$(PWD)/memalloc.so ./test

arena:
	g++ arena.cc -o arena
	./arena

clean:
	rm memalloc.so test arena
