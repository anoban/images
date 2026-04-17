COMPILER = /usr/bin/g++

DEBUG = -D_DEBUG -DDEBUG -O0 -g3

NODEBUG = -D_NDEBUG -DNDEBUG -O3 -g0

CFLAGS = -Wall -Wextra -D__TEST__ -I../include -I./googletest/include -I./googletest -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

GTEST_ALL = ./googletest/src/gtest-all.cc

SOURCES = ./src/*.cpp $(GTEST_ALL)

build:
	$(COMPILER) $(SOURCES) $(CFLAGS) $(NODEBUG) -o test.out

clean:
	rm -f ./*.out
	rm -f ./*.o
