#pragma once

#include <_internal.hpp>

#if !defined(__GNUG__) // will only work with LLVM, GCC and Intel oneAPI compiler, all of which define __GNUG__ on Linux systems
                       // appears that g++ gets confused about solitary quotes in #error messages
    #error \
        "LLVM and GCC SIMD intrinstics are liberally used in this project, except for the LLVM based Intel oneAPI compiler, other compilers probably won't work!"
#endif

#ifdef __INTEL_LLVM_COMPILER
    #define __CPP_COMPILER "Intel oneAPI"
#elif defined(__llvm__) && defined(__clang__)
    #define __CPP_COMPILER "LLVM"
#elif defined(__GNUG__)
    #define __CPP_COMPILER "GCC"
#endif

#undef __INTERNAL
