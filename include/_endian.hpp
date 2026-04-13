#pragma once
#ifndef __UTILITIES_HPP
    #error this header is not meant to be used outside of the utilities namespace
#endif

#include <cstdio>

#include <immintrin.h>

namespace endian {

    [[maybe_unused]] static constexpr inline unsigned short __attribute__((__always_inline__)) u16_from_be_bytes(
        const unsigned char* const bytestream
    ) noexcept {
        static_assert(sizeof(unsigned short) == 2);
        if (!bytestream) {
            ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
            return 0;
        }
        return static_cast<unsigned short>(bytestream[0]) << 8 | bytestream[1];
    }

    // WARNING :: WITH LLVM, DO NOT PASS BUFFERS SHORTER THAN 8 BYTES IN LENGTH
    [[maybe_unused]] static inline unsigned __attribute__((__always_inline__)) u32_from_be_bytes(const unsigned char* const bytestream) noexcept {
        static_assert(sizeof(unsigned) == 4);
        if (!bytestream) {
            ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
            return 0;
        }

#ifdef __llvm__ // LLVM defines __m64 as typedef __attribute__((__vector_size__(1 * sizeof(long long)))) long long __m64
        static constexpr __m64 mask_pi8 { 0x0405060700010203LL };
#elif defined(__GNUG__) // in GCC __m64 is typedef __attribute((vector_size(8))) int __m64
        // g++ bitches about a narrowing conversion here, "from long long int to int" but __m64 is in fact an 8 byte aligned vector BUT also an int???
        static const __m64 mask_pi8 {
            ::_mm_set_pi64x(0x0405060700010203LL) // ::_mm_set_pi64x() just does a simple type cast to __m64
        };
        // cannot constexpr this because ::_mm_set_pi64x() is not constexpr
#endif
        // move the first four bytes to the last four byte slot
        // even though only the first 4 bytes matter to this function, when reading in the stream of bytes as __m64, it'll dereference a sequence of 8 contiguous bytes

        return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];
    }

    [[maybe_unused]] static inline unsigned long long __attribute__((__always_inline__)) u64_from_be_bytes(const unsigned char* const bytestream) noexcept {
        static_assert(sizeof(unsigned long long) == 8);
        if (!bytestream) {
            ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
            return 0;
        }

#ifdef __llvm__
        static constexpr __m64 mask_pi8 { 0x01020304050607LL };
#elif defined(__GNUG__)
        static const __m64 mask_pi8 { ::_mm_set_pi64x(0x01020304050607LL) };
#endif
        return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];
    }

} // namespace endian
