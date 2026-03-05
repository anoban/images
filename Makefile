COMPILER = /usr/bin/g++

DEBUG = -DDEBUG -D_DEBUG -g3 -O0

NODEBUG = -D_NDEBUG -DNDEBUG -O3

CFLAGS = -Wall -Wextra  -I./include -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

TARGET = main

build:
	$(COMPILER) $(TARGET).cpp $(CFLAGS) $(NODEBUG) -o $(TARGET).out

clean:
	rm -f ./*.out
	rm -f ./*.o
