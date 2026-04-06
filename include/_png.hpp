#pragma once

// clang-format off
#include <_internal.hpp>
#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>
// clang-format on

#include <cstring>
#include <ctime>
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

// from the PNG specs, Table 7 - https://www.w3.org/TR/png-3/#table53

// critical chunks - (shall appear in this order, except for PLTE, which is optional)
// Chunk name 	Multiple allowed 	Ordering constraints
// IHDR 	    No 	                Shall be first
// PLTE 	    No 	                Before first IDAT
// IDAT 	    Yes                 Multiple IDAT chunks shall be consecutive
// IEND 	    No 	                Shall be last

// ancillary chunks - (need not to appear in this order)
// Chunk name 	Multiple allowed 	Ordering constraints
// acTL 	    No 	                Before IDAT
// cHRM 	    No 	                Before PLTE and IDAT
// cICP 	    No 	                Before PLTE and IDAT
// gAMA 	    No 	                Before PLTE and IDAT
// iCCP 	    No 	                Before PLTE and IDAT. If the iCCP chunk is present, the sRGB chunk should not be present.
// mDCV 	    No 	                Before PLTE and IDAT.
// cLLI 	    No 	                Before PLTE and IDAT.
// sBIT 	    No 	                Before PLTE and IDAT
// sRGB 	    No 	                Before PLTE and IDAT. If the sRGB chunk is present, the iCCP chunk should not be present.
// bKGD 	    No 	                After PLTE; before IDAT
// hIST 	    No 	                After PLTE; before IDAT
// tRNS 	    No 	                After PLTE; before IDAT
// eXIf 	    No 	                Before IDAT
// fcTL 	    Yes                 One may occur before IDAT; all others shall be after IDAT
// pHYs 	    No 	                Before IDAT
// sPLT 	    Yes                 Before IDAT
// fdAT 	    Yes                 After IDAT
// tIME 	    No 	                None
// iTXt 	    Yes                 None
// tEXt 	    Yes                 None
// zTXt 	    Yes                 None

class basic_chunk { // an opaque base class for all the implementations of PNG standard defined concrete chunk types
        // https://www.w3.org/TR/2025/REC-png-3-20250624/#table51
        // in the PNG standard, a chunk must have 3 or 4 essential fields, in this order
        // length
        // chunk type (name)
        // chunk data (optional, certain chunks may not have this field)
        // crc32 checksum

        // make sure every child class implements its own is_valid() member function which validates it's specific chunk requirements, so the ctor of png image class
        // can simply just call that method on all the chunk types to validate its construction

    protected:
        __TEST_ONLY(public)

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
            // for chunks that do not have a data segment, don't bother chunks with data
            if (!chunk.data) ostr << "| Data                " << std::setw(22) << "NULL|\n";
            ostr << std::dec;
            ostr << "| Checksum            " << std::setw(20) << chunk.checksum << "|\n";
            return ostr;
        }

    public:
        explicit inline basic_chunk(const unsigned char* const bytestream) noexcept :
            // this needs to be manually offsetted to beginnings of chunks in the png file buffer for the parsing to correctly take place
            length { internal::endian::u32_from_be_bytes(bytestream) },
            checksum { internal::endian::u32_from_be_bytes(internal::safe_offset(bytestream, sizeof(unsigned) + NAMELENGTH + length)) },
            checksum_buffer_beginning { internal::safe_offset(bytestream, sizeof(unsigned)) },
            data {
                length ?                                                               // if the length is non-zero
                    internal::safe_offset(bytestream, sizeof(unsigned) + NAMELENGTH) : // starts with the byte after the chunk name
                    nullptr                                                            // for zero length chunks
            },
            name {} {
            // copy the chunk name (second 4 bytes) into the local buffer
            ::strncpy(name, reinterpret_cast<const char*>(bytestream) + sizeof(unsigned), NAMELENGTH);
        }

        // since basic_chunk stores pointers raw pointers to the PNG file buffer, we do not want implicitly generated copy or move semantics
        basic_chunk() noexcept                              = delete;
        basic_chunk(const basic_chunk&) noexcept            = delete;
        basic_chunk& operator=(const basic_chunk&) noexcept = delete;
        basic_chunk(basic_chunk&&) noexcept                 = delete;
        basic_chunk& operator=(basic_chunk&&) noexcept      = delete;
        // because of this, all the derived classes will have these member functions implicitly deleted, which is great

        ~basic_chunk() noexcept                             = default;

        // compute the chunk's crc32 checksum and check whether it is same as the one stored in the chunk
        inline constexpr bool __attribute((__always_inline__)) is_checksum_valid() const noexcept {
            // for the crc32 checksum computation, we use the chunk's name and the bytes in the data segment
            return checksum == internal::crc::calculate(checksum_buffer_beginning, NAMELENGTH + length);
        }

        inline bool __attribute((__always_inline__)) is_name(const char* const str) const noexcept {
            return !::strncmp(name, str, NAMELENGTH); // ::strncmp returns 0 when the two null terminated strings are identical
        }
};

namespace critical { // IHDR, PLTE, IDAT & IEND are critical PNG chunks that must be present in all PNG images

    class ihdr final : public basic_chunk { // stands for Image HeaDeR, which is the first chunk in a PNG data stream
            // a PNG IHDR's data segment must contain the following, in the given order
            // Width 	            4 bytes
            // Height 	            4 bytes
            // Bit depth 	        1 byte
            // Color type 	        1 byte
            // Compression method 	1 byte
            // Filter method 	    1 byte
            // Interlace method 	1 byte

            __TEST_ONLY(public)

            unsigned           imwidth;  // width of a PNG image in pixels, 0 is an invalid value
            unsigned           imheight; // height of a PNG image in pixels, 0 is an invalid value
            unsigned char      bitdepth; // bit size of each colour channel value in the pixel
            COLOUR_TYPE        ctype;
            // only compression method 0 (deflate compression with a sliding window of at most 32768 bytes) is acceptable
            unsigned char      compression_method;
            // only filter method 0 (adaptive filtering with five basic filter types) is defined in the PNG specification
            unsigned char      filter_method;
            INTERLACING_METHOD interlace_method;

            constexpr bool __attribute((__always_inline__)) is_colourtype_bitdepth_invariants_valid() const noexcept {
                // https://www.w3.org/TR/png-3/#table111
                // PNG image type 	        Color type 	Allowed bit depths 	    Interpretation
                // Greyscale 	            0 	        1, 2, 4, 8, 16 	        Each pixel is a greyscale sample
                // Truecolor 	            2 	        8, 16 	                Each pixel is an R,G,B triple
                // Indexed-color 	        3 	        1, 2, 4, 8 	            Each pixel is a palette index; a PLTE chunk shall appear.
                // Greyscale with alpha 	4 	        8, 16 	                Each pixel is a greyscale sample followed by an alpha sample.
                // Truecolor with alpha 	6 	        8, 16 	                Each pixel is an R,G,B triple followed by an alpha sample.
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
                imwidth { internal::endian::u32_from_be_bytes(data) },                            // first 4 bytes of the data segment
                imheight { internal::endian::u32_from_be_bytes(internal::safe_offset(data, 4)) }, // second 4 bytes of data
                bitdepth { internal::safe_deref<unsigned char>(data, 8) },
                ctype { internal::safe_deref<unsigned char>(data, 9) },
                compression_method { internal::safe_deref<unsigned char>(data, 10) },
                filter_method { internal::safe_deref<unsigned char>(data, 11) },
                interlace_method { internal::safe_deref<unsigned char>(data, 12) } {
                //
            }

            inline constexpr bool __attribute((__always_inline__)) is_valid() const noexcept {
                return basic_chunk::is_checksum_valid() &&
                       basic_chunk::is_name("IHDR")
                       // 1) the data segment cannot be empty
                       && data
                       // 2) length must be 13
                       && (length == 13)
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

            __TEST_ONLY(public)
            // a PNG stream can only contain 1 PLTE chunk & the number of palette entries can range from 0 to 256
            // https://www.w3.org/TR/2025/REC-png-3-20250624/#11PLTE
            rgba     palette[MAX_PLTE_ENTRIES]; // each palette entry must be a 3 byte object (RGB)
            unsigned size_in_bytes;

        public:
            explicit inline plte(const unsigned char* const bytestream) noexcept : basic_chunk { bytestream } { }

            inline constexpr bool __attribute((__always_inline__)) is_valid(const COLOUR_TYPE& ctype) const noexcept {
                // length member stores the size of the palette entry array in bytes, an length not divisible by 3 without remainders is invalid
                return basic_chunk::is_checksum_valid() && basic_chunk::is_name("PLTE") && !(size_in_bytes % 3) &&
                       internal::is_in(internal::to_underlying(ctype), 2, 3, 6);
                // PLTE chunk shall appear for color type 3, and may appear for color types 2 and 6;
                // it shall not appear for color types 0 and 4.
                // there shall not be more than one PLTE chunk.
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

    class time final : public basic_chunk {
            // time stamp of the PNG image - gives the time of the last image modification (not the time of initial image creation)
            __TEST_ONLY(public)

            // tm tstamp; // using C's tm struct for convenience, but it's memory layout is not identical to PNG's tIME chunk's time encoding
            // the above turned out to be stupid idea as <stdio> functions operating on the tm struct expect it to be fully populated, PNG only encodes
            // certain members of that struct, with an incomplete struct passed, they spit out garbage

            // this is how a PNG stores a tIME chunk - https://www.w3.org/TR/png-3/#tIME-structure
            // Year 	2 bytes (complete; for example, 1995, not 95)
            // Month 	1 byte (1-12)
            // Day 	    1 byte (1-31)
            // Hour 	1 byte (0-23)
            // Minute 	1 byte (0-59)
            // Second 	1 byte (0-60) (to allow for leap seconds)
            // Universal Time (UTC) should be specified rather than local time

            unsigned short year;
            unsigned char  month;
            unsigned char  day;
            unsigned char  hour;
            unsigned char  minute;
            unsigned char  second;

        public:
            explicit inline time(const unsigned char* const chunkstream) noexcept :
                basic_chunk(chunkstream),
                year { internal::endian::u16_from_be_bytes(data) /* first 2 bytes */ },
                month { internal::safe_deref<unsigned char>(data, 2) },
                day { internal::safe_deref<unsigned char>(data, 3) },
                hour { internal::safe_deref<unsigned char>(data, 4) },
                minute { internal::safe_deref<unsigned char>(data, 5) },
                second { internal::safe_deref<unsigned char>(data, 6) } {
                    //
                };

            inline bool is_valid() const noexcept {
                return basic_chunk::is_name("tIME") && (length == 7) && internal::is_within_inclusive_range(month, 1, 12) &&
                       internal::is_within_inclusive_range(day, 1, 31) && internal::is_within_inclusive_range(hour, 0, 23) &&
                       internal::is_within_inclusive_range(minute, 0, 59) && internal::is_within_inclusive_range(second, 0, 60);
            }

            friend std::ostream& operator<<(std::ostream& ostr, const time& chunk) noexcept {
                ostr << static_cast<const basic_chunk&>(chunk);
                static char _buffer[128] {};

                ::memset(_buffer, 0, sizeof(_buffer)); // since our buffer is static, this is critical
                ::snprintf(
                    _buffer,
                    sizeof(_buffer),
                    "| Date                 %02d:%02d:%02d %02d-%02d-%04d|\n",
                    chunk.hour,
                    chunk.minute,
                    chunk.second,
                    chunk.day,
                    chunk.month,
                    chunk.year
                );
                // ostr << "| Date                 " << std::setw(2) << std::put_time(&chunk.tstamp, "%H:%M:%S %d-%m-%Y") << "|\n";
                ostr << _buffer;
                ostr << "-------------------------------------------\n";
                return ostr;
            }
    };

} // namespace ancillary

class png final {
        __TEST_ONLY(public)
        // critical chunks
        critical::ihdr header;
        critical::plte palette;
        critical::iend trailer;

        bool scan_and_parse() noexcept { }

        inline bool __is_valid() const noexcept {
            // https://www.w3.org/TR/2025/REC-png-3-20250624/#11PLTE
            // a PNG image can only have 1 PLTE chunk & it can only exists if the colour type is 2, 3 or 6
        }

    public:
        // explicit inline png(const char* const fpath) noexcept { }

        friend std::ostream& operator<<(std::ostream& ostr, const png& image) noexcept {
            //
            return ostr;
        }
};

#undef __INTERNAL
