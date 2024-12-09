#pragma once
#define __INTERNAL
#if !defined(__PNG) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <algorithm>

// project headers
#include <_helpers.hpp>
#include <_imageio.hpp>

#pragma comment(lib, "Ws2_32.lib")

// PNG (Portable Network Graphics) is a relatvely new image format compared to JPEG, BMP and GIF
// PNG uses loseless compression and supports:
// 1) up to 48 bits pixel depth
// 2) 1, 2, 4, 8, 16 bit sample precisions
// 3) alpha channel for full colour transparency
// 4) complicated colour matching ????

// PNG stores multibyte integers in MSB first order (Big Endian)
// bit strings are read from the least to the most significant bit ?? (related to Huffman codes), when a bit string steps over a byte boundary,
// bits in the second byte become the most significant ?? (Huffman codes)

// huffman codes within the compressed data are stored with their bits reversed.
// MSB of the Huffman codes will be the LSB of the data byte and vice versa.

// a PNG file is composed of a array of chunks.
// PNG chunks can be defined by three entities:
// 1) PNG standard - most important chunks all PNG decoders must be able of parse
// 2) public - a list of chunk specs submitted by the public and accepted by the PNG standard
// 3) private - chunks defined by applications e.g. Adobe PhotoShop (these chunks are often used internally by these applications)

// every PNG chunk consists of 4 parts:
// 1) length - number of bytes in the data segment, excluding the chunk itself (BE, unsigned)
// 2) type - chunk name/identifier (a string literal of 4 characters)
// 3) data - a series of bytes (contiguous array of length number of bytes)
// 4) CRC - a Cyclic Redundancy Check (CRC32) checksum value (BE, unsigned)

// a decoder only needs to parse standard defined PNG chunks that follow the above structure
// it shall ignore chunks that are not in compliance with the above specified format, private chunks and newly incorporated public chunks

class png_chunk final {
        unsigned       length;
        char           type[4]; // NOLINT(modernize-avoid-c-arrays)
        unsigned char* data;
        unsigned       crc32_checksum;
        // crc32 checksum is computed for all bytes of a given chunk following the length field (the first four bytes are excluded)
        // a crc32 checksum must always be there even for chunks with an empty data field

    public:
        constexpr bool is_critical() const noexcept {
            // a chunk with uppercase first letter is considered a critical chunk, all PNG decoders must be able to parse the critical chunks
            return ascii::is_uppercase(type[0]);
        }

        constexpr bool is_valid() const noexcept {
            // any chunk with non printing ASCII characters in its name must be deemed invalid
            // first, second and the fourth character in the name can be an uppercase or a lowercase letter
            // third character MUST be an uppercase ASCII letter
            return ascii::is_alphabet_array(type) && ascii::is_uppercase(type[2]);
        }

        constexpr bool is_private() const noexcept {
            // second character of public chunk types and chunk types approved by the PNG development group will be in uppercase
            // private chunks defined by applications must have a lowercase second character, in order not to conflict with publicly defined chunks
            return ascii::is_lowercase(type[1]);
        }

        // if a chunk is safe to copy, the last alphabet of the chunk name shall be in lowercase (NOT ALWAYS)
        // a decoder shall not attempt to copy a non-copy-safe chunk if it doesn't recognize it
        // i.e there are chunks with uppercase last letter that are safe to copy, like the PNG standards defined IHDR, IDAT, IEND ... chunks
        // this lowercase rule applies as a fallback only when the decoder cannot recognize a chunk
};

// every PNG data stream must begin with the bytes :: {137, 'P', 'N', 'G', 13, 10, 26, 10}
static constexpr std::array<unsigned char, 8> PNG_SIGNATURE { 10, 26, 10, 13, 'G', 'N', 'P', 137 };

enum class PIXEL_KIND : unsigned char { GREYSCALE = 0, RGBTRIPLE = 2, COLOURPALETTE = 3, GREYSCALEALPHA = 4, RGBALPHA = 6 };

enum class INTERLACING_KIND : unsigned char { NONE /* not interlaced */, ADAM7 /* uses Adam 7 interlacing */ };

enum class CHUNK_KIND : unsigned char { IHDR, PLTE, IDAT, IEND, bKGD /* background */, cHRM, gAMA, UNRECOGNIZED };

class IHDR final {                  // layout of the IHDR chunk data, i.e the data buffer of an IHDR chunk should be in the following format
        unsigned         width;     // width in pixels
        unsigned         height;    // height in pixels
        unsigned char    bit_depth; // can be 1, 2, 4, 8 or 16
        PIXEL_KIND       colour_type;
        unsigned char    compression_method; // must be 0
        unsigned char    filter_method;      // must be 0
        INTERLACING_KIND interlace_method;

    public:
        IHDR(_In_reads_bytes_(size) const unsigned char* const chunkdata, _In_ const unsigned long size) noexcept :
            width { endian::ulong_from_be_bytes(chunkdata) },
            height { endian::ulong_from_be_bytes(chunkdata + 4) },
            bit_depth { *(chunkdata + 8) },
            colour_type { *(chunkdata + 9) },
            compression_method { *(chunkdata + 10) },
            filter_method { *(chunkdata + 11) },
            interlace_method { *(chunkdata + 12) } {
            assert(size >= sizeof(IHDR)); // PROBLEMATIC
        }

        ~IHDR() noexcept = default;

        bool is_valid() const noexcept { return width && height; }
};

class PLTE final { };

class IDAT final { };

class cHRM final { // layout of the cHRM chunk data
        unsigned white_x;
        unsigned white_y;
        unsigned red_x;
        unsigned red_y;
        unsigned green_x;
        unsigned green_y;
        unsigned blue_x;
        unsigned blue_y;
};

class gAMA final { // layout of the gAMA chunk data
        unsigned gamma;
};

// the PLTE chunks (colour palettes) in PNG images use the RGBTRIPLE pixel format, also known as "palette entries"
// unlike Windows bitmaps with B, G & R sequenced pixels, the colour values are ordered in the same sequence implied by the name - R, G & B
struct RGB final {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
};

namespace png_chunk_names { // there are 4 critical chunks each PNG file is expected to contain - IHDR, PLTE, IDAT and IEND
    static constexpr std::array<char, 4> IHDR { 'I', 'H', 'D', 'R' }; // image header, the first chunk in a PNG data stream
    static constexpr std::array<char, 4> PLTE { 'P', 'L', 'T', 'E' }; // palette table
    static constexpr std::array<char, 4> IDAT { 'I', 'D', 'A', 'T' }; // image chunk
    static constexpr std::array<char, 4> IEND { 'I', 'E', 'N', 'D' }; // image trailer, the last chunk in a PNG data stream
} // namespace png_chunk_names

// this overloaded operator== is only intended to compare PNG chunk names!
template<unsigned long long length>
static constexpr typename std::enable_if<length == 4LLU, bool>::type operator==( // NOLINT(modernize-use-constraints)
    _In_ const char (&left)[length],                                             // NOLINT(modernize-avoid-c-arrays)
    _In_ const std::array<char, length>& right
) noexcept {
    return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

template<unsigned long long length>
static constexpr typename std::enable_if<length == 4LLU, bool>::type operator==( // NOLINT(modernize-use-constraints)
    _In_ const std::array<char, length>& left,
    _In_ const char                      (&right)[length] // NOLINT(modernize-avoid-c-arrays)
) noexcept {
    return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

class png final {
    private:

    public:
        png() noexcept = default;

        explicit png(_In_ const wchar_t* const filepath) noexcept { }

        bool decode() noexcept { }

        bool encode() noexcept { }
};

#undef __INTERNAL
