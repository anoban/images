COMPILER = /usr/bin/clang++

DBG_FLAGS = -DDEBUG -D_DEBUG -g3

CFLAGS = -Wall -Wextra -O3 -D_NDEBUG -DNDEBUG -I./include -std=c++20 -march=native -mavx512f

TARGET = main

build:
	$(COMPILER) $(CFLAGS) -o main.out

clean:
	rm -f ./*.out
	rm -f ./*.o
