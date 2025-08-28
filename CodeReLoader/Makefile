test:
	mkdir -p build
	g++ -shared test/test.cc -o build/test.o

build: test
	mkdir -p build
	g++ driver.cc -o build/sim

clean:
	rm -rf build

.PHONY: test build clean
