#pragma once
#define __INTERNAL
#if !defined(__IMAGEIO) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>

// https://www.w3.org/TR/png-3/
class png final {
    private:
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static constexpr unsigned char      PNG_SIGNATURE[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        static constexpr unsigned long long PNG_CHUNK_NAME_LENGTH { 4 };

    public:
        class basic_chunk {
            private:
                unsigned       length; // first four bytes of a PNG chunk, documents the number of bytes in the data segment of the chunk
                char           name[PNG_CHUNK_NAME_LENGTH]; // NOLINT(modernize-avoid-c-arrays)
                unsigned char* data;
                unsigned       crc;

            public:
                explicit basic_chunk(_In_ const unsigned char* const pngstream) noexcept : length {}, name {}, data {}, crc {} { }
        };

    public:
};

#undef __INTERNAL
