#pragma once

// clang-format off
#include <_internal.hpp>
#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>
// clang-format on

#include <cstring>
#include <iomanip>

// look up https://www.w3.org/TR/png-3/ for the official PNG specification document

struct rgb final {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
};

struct rgba final {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
};

static constexpr unsigned char SIGNATURE[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
static constexpr unsigned long NAMELENGTH { 4 };
static constexpr unsigned long MAX_PLTE_ENTRIES { 256 };

enum COLOUR_TYPE : unsigned char { // lookup table 12 in the PNG spec
    GREYSCALE             = 0,     // each pixel is a greyscale sample, allowed bit depths - 1, 2, 4, 8, 16
    TRUECOLOUR            = 2,     // each pixel is an RGB triple, allowed bit depths - 8, 16
    INDEXED_COLOUR        = 3,     // each pixel is a palette index; image must contain a PLTE chunk, allowed bit depths - 1, 2, 4, 8
    GREYSCALE_WITH_ALPHA  = 4,     // each pixel is a greyscale sample followed by an alpha sample, alowed bit depths - 8, 16
    TRUECOLOUR_WITH_ALPHA = 6      // each pixel is an RGB triple followed by an alpha sample (RGBA), alowed bit depths - 8, 16
};

enum INTERLACING_METHOD : unsigned char { NONE = 0, ADAM7 = 1 };

class basic_chunk { // an opaque base class for all the implementations of PNG standard defined concrete chunk types
        // https://www.w3.org/TR/2025/REC-png-3-20250624/#table51
        // in the PNG standard, a chunk must have 3 or 4 essential fields
        // 1) length
        // 2) chunk type (name)
        // 3) chunk data (optional, certain chunks may not have this field)
        // 4) crc32 checksum

    protected:
        _TEST_ACCESS(public:)

        unsigned             length; // first four bytes of a PNG chunk, documents the number of bytes in the data segment of the chunk
        // length accounts only for the bytes in the data segment and excludes itself, chunk name and the crc checksum
        unsigned             checksum;                  // crc32 checksum of chunk name + chunk data i.e (length + 4) bytes
        const unsigned char* checksum_buffer_beginning; // pointer to the stream of bytes after the first 4 bytes
        const unsigned char* data;                      // the actual chunk data (length bytes long)
        char                 name[NAMELENGTH + 1];      // PNG chunk name made of 4 ascii characters and a null terminator

        // this operator<< isn't meant to be called directly in places other than derived class member functions
        friend std::ostream& operator<<(std::ostream& ostr, const basic_chunk& chunk) noexcept {
            ostr << "-------------------------------------------\n";
            ostr << "| Length              " << std::setw(20) << chunk.length << "|\n";
            ostr << "| Name                " << std::setw(20) << chunk.name << "|\n";
            if (chunk.data)
                ostr << "| Data                " << std::setw(20) << std::hex << std::uppercase << chunk.data << "|\n";
            else
                ostr << "| Data                " << std::setw(22) << "NULL|\n"; // for chunks that do not have a data segment
            ostr << std::dec;
            ostr << "| Checksum            " << std::setw(20) << chunk.checksum << "|\n";
            return ostr;
        }

    public:
        // this constructor doubles as a parser too
        explicit inline basic_chunk(const unsigned char* const bytestream) noexcept :
            // this needs to be manually offsetted to beginnings of chunks in the png file buffer for the parsing to correctly take place
            length { internal::endian::u32_from_be_bytes(bytestream) },
            checksum { internal::endian::u32_from_be_bytes(bytestream + sizeof(unsigned) + NAMELENGTH + length) },
            checksum_buffer_beginning { bytestream + sizeof(unsigned) },
            data { length /* if the length is non-zero */ ?
                       bytestream + sizeof(unsigned) + NAMELENGTH /* starts with the byte after the chunk name */ :
                       nullptr /* for zero length chunks */ },
            name {} {
            // copy the chunk name (second 4 bytes) into the local buffer
            ::strncpy(name, reinterpret_cast<const char*>(bytestream) + sizeof(unsigned), NAMELENGTH);
        }

        // since basic_chunk stores pointers TO THE PNG FILE BUFFER INDICATING THE BEGINNING OF THE DATA SEGMENT, WE DO NOT WANT IMPLICITLY ENABLED COPY OR MOVE SEMANTICS
        basic_chunk() noexcept                              = delete;
        basic_chunk(const basic_chunk&) noexcept            = delete;
        basic_chunk& operator=(const basic_chunk&) noexcept = delete;
        basic_chunk(basic_chunk&&) noexcept                 = delete;
        basic_chunk& operator=(basic_chunk&&) noexcept      = delete;
        // BECAUSE OF THIS, ALL THE DERIVED CHUNK TYPES WILL HAVE THESE MEMBER FUNCTIONS IMPLICITLY DELETED, COOL :)

        ~basic_chunk() noexcept                             = default;

        // compute the chunk's CRC32 checksum and check whether it is same as the one stored in the chunk
        inline constexpr bool __attribute((__always_inline__)) is_checksum_valid() const noexcept {
            // for the CRC32 checksum computation, we use the chunk's name and the bytes in the data segment
            return checksum == internal::crc::calculate(checksum_buffer_beginning, NAMELENGTH + length);
        }

        inline constexpr bool __attribute((__always_inline__)) is_name(const char* const str) const noexcept {
            // return std::equal(str, str + NAMELENGTH, name);
            return !::strncmp(name, str, NAMELENGTH); // ::strncmp returns 0 when the two null terminated strings are identical
        }
};

namespace critical { // IHDR, PLTE, IDAT & IEND are critical PNG chunks that must be present in all PNG images

    class ihdr final : public basic_chunk { // stands for Image HeaDeR, which is the first chunk in a PNG data stream

            _TEST_ACCESS(public:)

            unsigned           imwidth;  // width of a PNG image in pixels, 0 is an invalid value
            unsigned           imheight; // height of a PNG image in pixels, 0 is an invalid value
            unsigned char      bitdepth; // bit size of each colour channel value in the pixel
            COLOUR_TYPE        ctype;
            // only compression method 0 (deflate compression with a sliding window of at most 32768 bytes) is acceptable
            unsigned char      compression_method;
            // only filter method 0 (adaptive filtering with five basic filter types) is defined in the PNG specification
            unsigned char      filter_method;
            INTERLACING_METHOD interlace_method;

            [[nodiscard]] constexpr bool __attribute((__always_inline__)) is_colourtype_bitdepth_invariants_valid() const noexcept {
                switch (ctype) {
                    case COLOUR_TYPE::GREYSCALE             : return internal::is_in(bitdepth, 1, 2, 4, 8, 16);
                    case COLOUR_TYPE::INDEXED_COLOUR        : return internal::is_in(bitdepth, 1, 2, 4, 8);
                    case COLOUR_TYPE::TRUECOLOUR            : [[fallthrough]]; // the following three require the same criterion for validity
                    case COLOUR_TYPE::GREYSCALE_WITH_ALPHA  : [[fallthrough]];
                    case COLOUR_TYPE::TRUECOLOUR_WITH_ALPHA : return internal::is_in(bitdepth, 8, 16);
                    default                                 : [[unlikely]] return false;
                }
            }

        public:
            explicit inline ihdr(const unsigned char* const bytestream) noexcept :
                basic_chunk { bytestream },
                imwidth { internal::endian::u32_from_be_bytes(data) },      // first 4 bytes of the data segment
                imheight { internal::endian::u32_from_be_bytes(data + 4) }, // second 4 bytes of data
                bitdepth { *(data + 8) },
                ctype { *(data + 9) },
                compression_method { *(data + 10) },
                filter_method { *(data + 11) },
                interlace_method { *(data + 12) } {
                //
            }

            inline constexpr bool __attribute((__always_inline__)) is_valid() const noexcept {
                return basic_chunk::is_checksum_valid() &&
                       basic_chunk::is_name("IHDR")
                       // 1) the data segment cannot be empty
                       && data
                       // 2) length must be 13
                       && (length == 13U)
                       // 3) height and width cannot be 0
                       && imheight &&
                       imwidth
                       // 4) combinations of colour type and bit depth must be valid
                       && is_colourtype_bitdepth_invariants_valid()
                       // 5) compression method must be 0
                       && !compression_method
                       // 6) filtering method must be 0
                       && !filter_method
                       // 7) interlacing method must be 0 or 1
                       && (interlace_method == INTERLACING_METHOD::NONE || interlace_method == INTERLACING_METHOD::ADAM7);
            }

            friend std::ostream& operator<<(std::ostream& ostr, const ihdr& header) noexcept {
                ostr << static_cast<const basic_chunk&>(header);
                ostr << "| Width               " << std::setw(20) << header.imwidth << "|\n";
                ostr << "| Height              " << std::setw(20) << header.imheight << "|\n";
                ostr << "| Bit depth           " << std::setw(20) << static_cast<int>(header.bitdepth) << "|\n";
                ostr << "| Colour type         " << std::setw(20) << static_cast<int>(header.ctype) << "|\n";
                ostr << "| Compression method  " << std::setw(20) << static_cast<int>(header.compression_method) << "|\n";
                ostr << "| Filter method       " << std::setw(20) << static_cast<int>(header.filter_method) << "|\n";
                ostr << "| Interlace method    " << std::setw(20) << static_cast<int>(header.interlace_method) << "|\n";
                ostr << "-------------------------------------------\n";
                return ostr;
            }
    };

    class plte final : public basic_chunk { // stands for PaLeTtE, contains an array of colour entries (RGB)

            _TEST_ACCESS(public:)
            // a PNG stream can only contain 1 PLTE chunk & the number of palette entries can range from 0 to 256
            // https://www.w3.org/TR/2025/REC-png-3-20250624/#11PLTE
            rgb      palette[MAX_PLTE_ENTRIES]; // each palette entry must be a 3 byte object (RGB)
            // length member stores the size of the palette entry array in bytes, an length not divisible by 3 without remainders is invalid
            unsigned size_in_bytes;

        public:
            explicit inline plte(const unsigned char* const bytestream) noexcept : basic_chunk { bytestream } { }

            inline constexpr bool __attribute((__always_inline__)) is_valid() const noexcept {
                //
                return basic_chunk::is_checksum_valid() && basic_chunk::is_name("PLTE") && !(size_in_bytes % 3);
            }

            [[deprecated]] friend std::ostream& operator<<(std::ostream& ostr, const plte& chunk) noexcept {
                ostr << static_cast<const basic_chunk&>(chunk);
                ostr << "-------------------------------------------\n";
                return ostr;
            }
    };

    class idat final : public basic_chunk { // stands for Image DATa
    };

    class iend final : public basic_chunk { // image trailer, the last chunk in a PNG data stream
        public:
            explicit iend(const unsigned char* const chunkstream) noexcept : basic_chunk(chunkstream) { }

            inline constexpr bool __attribute((__always_inline__)) is_valid() const noexcept {
                // IEND chunk's data field must be empty and the length must be 0
                return basic_chunk::is_checksum_valid() && basic_chunk::is_name("IEND") && !data && !length;
            }

            friend std::ostream& operator<<(std::ostream& ostr, const iend& chunk) noexcept {
                ostr << static_cast<const basic_chunk&>(chunk); // IEND doesn't have any special data members, so this is adequate
                ostr << "-------------------------------------------\n";
                return ostr;
            }
    };

} // namespace critical

namespace ancillary {

    // TRANSPARENCY INFORMATION

    class tRNS final : public basic_chunk { };

    // COLOURSPACE INFORMATION

    class cHRM final : public basic_chunk { };

    class gAMA final : public basic_chunk { };

    class sRGB final : public basic_chunk { };

    // TEXTUAL INFORMATION

    class tEXt final : public basic_chunk { };

    class zTXt final : public basic_chunk { };

    class iTXt final : public basic_chunk { };

    // MISCELLANEOUS INFORMATION

    class bKGD final : public basic_chunk { };

    class hIST final : public basic_chunk { };

    class pHYs final : public basic_chunk { };

    class time final : public basic_chunk { // time stamp of the PNG image
            _TEST_ACCESS(public:)

            unsigned short year;   // in YYYY format; for example, 1995, not 95
            unsigned char  month;  // 1-12
            unsigned char  day;    // 1-31
            unsigned char  hour;   // 0-23
            unsigned char  minute; // 0-59
            unsigned char  second; // 0-60, to allow for leap seconds

        public:
            explicit inline time(const unsigned char* const chunkstream) noexcept :
                basic_chunk(chunkstream),
                year { ::__bswap_16(*reinterpret_cast<const unsigned short*>(data)) },
                month { *(data + 1) },
                day { *(data + 2) },
                hour { *(data + 3) },
                minute { *(data + 4) },
                second { *(data + 5) } {
                    //
                };
    };

} // namespace ancillary

class png final {
        _TEST_ACCESS(public:)
        // critical chunks
        critical::ihdr image_header;
        critical::plte colour_palette;
        critical::iend image_trailer;

        [[nodiscard]] bool scan_and_parse() noexcept { }

        inline bool is_invariants_valid() const noexcept {
            // https://www.w3.org/TR/2025/REC-png-3-20250624/#11PLTE
            // a PNG image can only have 1 PLTE chunk & it can only exists if the colour type is 2, 3 or 6
        }

    public:
        friend std::ostream& operator<<(std::ostream& ostr, const png& image) noexcept {
            //
            return ostr;
        }
};

#undef __INTERNAL
