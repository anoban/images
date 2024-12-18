#pragma once
#define __INTERNAL
#if !defined(__IMAGEIO) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <iomanip>

#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>

// look up https://www.w3.org/TR/png-3/ for the official PNG specification document

// an RGB pixel struct with colour values in the traditional sequence unlike the BGR sequenced Windows pixel structs
/*
    typedef struct tagRGBTRIPLE {  // choosing NOT to leverage Wingdi here
        BYTE rgbtBlue;
        BYTE rgbtGreen;
        BYTE rgbtRed;
    } RGBTRIPLE, *PRGBTRIPLE, *NPRGBTRIPLE, *LPRGBTRIPLE;
*/
struct rgbtriple final {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
};

namespace internal {
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr unsigned char      PNG_SIGNATURE[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    static constexpr unsigned long long PNG_CHUNK_NAME_LENGTH { 4 };

    class basic_chunk { // an opaque base class for all the implementations of PNG standard defined concrete chunk types

        protected:
            // clang-format off
#ifdef __TEST__
        public:
#endif
            // clang-format on
            unsigned long        length; // first four bytes of a PNG chunk, documents the number of bytes in the data segment of the chunk
                // length accounts only for the bytes in the data segment and excludes itself, chunk name and the checksum
                // imagine the data member is written as if `unsigned char data[length];`
            const unsigned char* name;     // NOLINT(modernize-avoid-c-arrays), PNG chunk name made of 4 ASCII characters
            const unsigned char* data;     // the actual chunk data (length bytes long)
            unsigned long        checksum; // CRC32 checksum of chunk name + chunk data i.e (length + 4) bytes

            friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const basic_chunk& chunk) noexcept {
                wostr << L"-------------------------------------------\n";
                wostr << L"| Length              " << std::setw(20) << chunk.length << L"|\n";
                wostr << L"| Name                " << std::setw(17) << static_cast<wchar_t>(chunk.name[0])
                      << static_cast<wchar_t>(chunk.name[1]) << static_cast<wchar_t>(chunk.name[2]) << static_cast<wchar_t>(chunk.name[3])
                      << L"|\n";
                wostr << L"| Data                " << std::setw(20) << std::hex << std::uppercase << chunk.data << L"|\n";
                wostr << std::dec;
                wostr << L"| Checksum            " << std::setw(20) << chunk.checksum << L"|\n";

                return wostr;
            }

        public:
            explicit basic_chunk( // this constructor doubles as a parser too
                _In_ const unsigned char* const
                    pngstream /* this needs to be manually offsetted to beginnings of chunks in the png file buffer for the parsing to correctly take place */
            ) noexcept : // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                length { internal::endian::ulong_from_be_bytes(pngstream) },
                name { pngstream + sizeof(unsigned) },
                data { length /* if the length is non-zero */ ? pngstream + sizeof(unsigned) + PNG_CHUNK_NAME_LENGTH /* name */ : nullptr },
                checksum { internal::endian::ulong_from_be_bytes(pngstream + sizeof(unsigned) + PNG_CHUNK_NAME_LENGTH + length) } {
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }

            // SINCE PNG CHUNKS STORE A POINTER TO THE PNG FILE BUFFER INDICATING THE BEGINNING OF THE DATA SEGMENT, WE DO NOT WANT IMPLICITLY ENABLED COPY OR MOVE SEMANTICS
            basic_chunk() noexcept                              = delete;
            basic_chunk(const basic_chunk&) noexcept            = delete;
            basic_chunk& operator=(const basic_chunk&) noexcept = delete;
            basic_chunk(basic_chunk&&) noexcept                 = delete;
            basic_chunk& operator=(basic_chunk&&) noexcept      = delete;
            // BECAUSE OF THIS, ALL THE DERIVED CHUNK TYPES WILL HAVE THESE MEMBER FUNCTIONS IMPLICITLY DELETED, COOL :)

            ~basic_chunk() noexcept                             = default;

            // compute the chunk's CRC32 checksum and check whether it is same as the one stored in the chunk
            [[nodiscard]] bool __stdcall is_checksum_valid() const noexcept {
                return checksum ==
                       internal::crc::get(
                           name, // for the CRC32 checksum computation, we use the chunk's name and the bytes in the data segment
                           PNG_CHUNK_NAME_LENGTH + length
                       );
            }

            [[nodiscard]] bool __stdcall is_name(_In_reads_(PNG_CHUNK_NAME_LENGTH) const char* const str) const noexcept {
                return std::equal(str, str + PNG_CHUNK_NAME_LENGTH, name); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
    };

    // IHDR, PLTE, IDAT & IEND are critical PNG chunks that must be present in all PNG images
    // IHDR stands for Image HeaDeR, which is the first chunk in a PNG data stream
    class IHDR final : public basic_chunk {
        public:
            // LOOKUP TABLE 12 IN THE PNG SPECIFICATION
            enum PNG_COLOUR_TYPE : unsigned char {
                GREYSCALE             = 0, // each pixel is a greyscale sample, allowed bit depths - 1, 2, 4, 8, 16
                TRUECOLOUR            = 2, // each pixel is an RGB triple, allowed bit depths - 8, 16
                INDEXED_COLOUR        = 3, // each pixel is a palette index; a PLTE chunk shall appear, allowed bit depths - 1, 2, 4, 8
                GREYSCALE_WITH_ALPHA  = 4, // each pixel is a greyscale sample followed by an alpha sample, alowed bit depths - 8, 16
                TRUECOLOUR_WITH_ALPHA = 6  // each pixel is an R,G,B triple followed by an alpha sample, alowed bit depths - 8, 16
            };

            enum PNG_INTERLACING_METHOD : unsigned char { NONE = 0, ADAM7 = 1 };

            // clang-format off
#ifndef __TEST__
        private: // IHDR specific data
#endif
            // clang-format on
            unsigned long          width;  // width of a PNG image in pixels, 0 is an invalid value
            unsigned long          height; // height of a PNG image in pixels, 0 is an invalid value
            unsigned char          bit_depth;
            PNG_COLOUR_TYPE        colour_type;
            // only compression method 0 (deflate compression with a sliding window of at most 32768 bytes) is valid
            unsigned char          compression_method;
            // only filter method 0 (adaptive filtering with five basic filter types) is defined in the PNG specification
            unsigned char          filter_method;
            PNG_INTERLACING_METHOD interlace_method;

            [[nodiscard]] bool __stdcall is_colourtype_bitdepth_invariants_valid() const noexcept {
                switch (colour_type) {
                    case PNG_COLOUR_TYPE::GREYSCALE             : return internal::is_in(bit_depth, 1, 2, 4, 8, 16);
                    case PNG_COLOUR_TYPE::INDEXED_COLOUR        : return internal::is_in(bit_depth, 1, 2, 4, 8);
                    case PNG_COLOUR_TYPE::TRUECOLOUR            : [[fallthrough]]; // the following three require the same criterion for validity
                    case PNG_COLOUR_TYPE::GREYSCALE_WITH_ALPHA  : [[fallthrough]];
                    case PNG_COLOUR_TYPE::TRUECOLOUR_WITH_ALPHA : return internal::is_in(bit_depth, 8, 16);
                    default                                     : [[unlikely]] return false;
                }
            }

        public:
            explicit IHDR(_In_ const unsigned char* const chunkstream) noexcept :
                basic_chunk(chunkstream),
                width { internal::endian::ulong_from_be_bytes(data) },
                // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                height { internal::endian::ulong_from_be_bytes(data + 4) },
                bit_depth { *(data + 8) },
                colour_type { *(data + 9) },
                compression_method { *(data + 10) },
                filter_method { *(data + 11) },
                interlace_method { *(data + 12) } {
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }

            [[nodiscard]] bool __stdcall is_valid() const noexcept {
                // for an IHDR chunk
                return basic_chunk::is_checksum_valid() &&
                       basic_chunk::is_name("IHDR")
                       // 1) the data segment cannot be empty
                       && data
                       // 2) length must be 13
                       && (length == 13U)
                       // 3) height and width cannot be empty
                       && height &&
                       width
                       // 4) combinations of colour type and bit depth must be valid
                       && is_colourtype_bitdepth_invariants_valid()
                       // 5) compression method must be 0
                       && !compression_method
                       // 6) filtering method must be 0
                       && !filter_method
                       // 7) interlacing method must be 0 or 1
                       && (interlace_method == PNG_INTERLACING_METHOD::NONE || interlace_method == PNG_INTERLACING_METHOD::ADAM7);
            }

            friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const IHDR& header) noexcept {
                wostr << static_cast<const basic_chunk&>(header);
                wostr << L"| Width               " << std::setw(20) << header.width << L"|\n";
                wostr << L"| Height              " << std::setw(20) << header.height << L"|\n";
                wostr << L"| Bit depth           " << std::setw(20) << header.bit_depth << L"|\n";
                wostr << L"| Colour type         " << std::setw(20) << header.colour_type << L"|\n";
                wostr << L"| Compression method  " << std::setw(20) << header.compression_method << L"|\n";
                wostr << L"| Filter method       " << std::setw(20) << header.filter_method << L"|\n";
                wostr << L"| Interlace method    " << std::setw(20) << header.interlace_method << L"|\n";
                wostr << L"-------------------------------------------\n";
                return wostr;
            }

            ~IHDR() noexcept = default;
    };

    class PLTE final : public basic_chunk { };

    // IDAT stands for Image DATa
    class IDAT final : public basic_chunk { };

    // image trailer, the last chunk in a PNG data stream
    class IEND final : public basic_chunk {
        public:
            explicit IEND(_In_ const unsigned char* const chunkstream) noexcept : basic_chunk(chunkstream) { }

            [[nodiscard]] bool __stdcall is_valid() const noexcept {
                // IEND chunk's data filed must be empty and the length must be 0
                return basic_chunk::is_checksum_valid() && basic_chunk::is_name("IEND") && !data && !length;
            }

            friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const IEND& chunk) noexcept {
                wostr << static_cast<const basic_chunk&>(chunk); // IEND doesn't have any special data members, so this is adequate
                wostr << L"-------------------------------------------\n";
                return wostr;
            }

            ~IEND() noexcept = default;
    };

} // namespace internal

class png final {
    public:
        // clang-format off
#ifndef __TEST__
    private:
#endif
        // clang-format on

        [[nodiscard]] bool __stdcall scan_and_parse() noexcept { }

        // critical chunks

    public:
        friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const png& image) noexcept {
            //
            return wostr;
        }
};

#undef __INTERNAL
