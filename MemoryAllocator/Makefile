build:
	gcc -o memalloc.so -fPIC -shared memAlloc.c

test: build
	gcc test.c -o test
	LD_PRELOAD=$(PWD)/memalloc.so ./test

clean:
	rm memalloc.so test
