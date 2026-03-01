COMPILER = /usr/bin/clang

CFLAGS = -Wall -Wextra -O3 -D_NDEBUG -DNDEBUG -I./include -std=c++20

TARGET = main

build:
	$(COMPILER)

clean:
	rm -f ./*.out
	rm -f ./*.o
