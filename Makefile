COMPILER = /usr/bin/clang

DBG_FLAGS = -DDEBUG -D_DEBUG -g3

CFLAGS = -Wall -Wextra -O3 -D_NDEBUG -DNDEBUG -I./include -std=c++20

TARGET = main

build:
	$(COMPILER)

clean:
	rm -f ./*.out
	rm -f ./*.o
