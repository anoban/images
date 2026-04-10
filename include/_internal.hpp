#pragma once
#define __INTERNAL 1

#if !defined(__INTERNAL) && !defined(__TEST__)
    #error Do not directly include headers prefixed with an underscore in source files, use the unprefixed variants without the .hpp extension instead.
#endif

#pragma once

#if !defined(__GNUG__) // will only work with LLVM, GCC and Intel oneAPI compiler, all of which define __GNUG__ on Linux systems
                       // appears that g++ gets confused about solitary quotes in #error messages
    #error \
        "LLVM and GCC SIMD intrinstics are liberally used throughout this project, except for the LLVM based Intel oneAPI compiler, other compilers probably won't work!"
#endif

#ifdef __INTEL_LLVM_COMPILER
    #define __CPP_COMPILER "Intel oneAPI"
#elif defined(__llvm__) && defined(__clang__)
    #define __CPP_COMPILER "LLVM"
#elif defined(__GNUG__)
    #define __CPP_COMPILER "GCC"
#endif

#define __STDC_WANT_LIB_EXT1__ 1

#include <endian.h>

#if __BYTE_ORDER != __LITTLE_ENDIAN
    #error This codebase assumes little endian hardware and will not run correctly on big endian hardware!
#endif
