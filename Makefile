INTEL = /opt/intel/oneapi/compiler/latest/bin/icpx

GCC = /usr/bin/g++

CLANG = /usr/bin/clang++

DEBUG = -DDEBUG -D_DEBUG -g3 -O0

NODEBUG = -D_NDEBUG -DNDEBUG -O3 -g0

CFLAGS = -Wall -Wextra  -I./include -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

TARGET = main

build:
	$(CLANG) $(TARGET).cpp $(CFLAGS) $(NODEBUG) -o $(TARGET).out

clean:
	rm -f ./*.out
	rm -f ./*.o
