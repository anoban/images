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

namespace internal {
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr unsigned char      PNG_SIGNATURE[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    static constexpr unsigned long long PNG_CHUNK_NAME_LENGTH { 4 };

    class basic_chunk { // an opaque base class for all the implementations of PNG standard defined concrete chunk types
        protected:
            unsigned long        length; // first four bytes of a PNG chunk, documents the number of bytes in the data segment of the chunk
                // length accounts only for the bytes in the data segment and excludes itself, chunk name and the checksum
                // imagine the data member is written as if `unsigned char data[length];`
            const unsigned char* name;     // NOLINT(modernize-avoid-c-arrays), PNG chunk name made of 4 ASCII characters
            const unsigned char* data;     // the actual chunk data (length bytes long)
            unsigned long        checksum; // CRC32 checksum of chunk name + chunk data i.e (length + 4) bytes

        public:
            explicit basic_chunk(
                _In_ const unsigned char* const
                    pngstream /* this needs to be manually offsetted to beginnings of chunks in the png file buffer for the parsing to correctly take place */
            ) noexcept : // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                length { internal::endian::ulong_from_be_bytes(pngstream) },
                name { pngstream + sizeof(length) },
                data { length /* if the length is non-zero */ ? pngstream + sizeof(length) + PNG_CHUNK_NAME_LENGTH /* name */ : nullptr },
                checksum { internal::endian::ulong_from_be_bytes(pngstream + sizeof(length) + PNG_CHUNK_NAME_LENGTH + length) } {
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }

            // SINCE PNG CHUNKS STORE A POINTER TO THE PNG FILE BUFFER INDICATING THE BEGINNING OF THE DATA SEGMENT, WE DO NOT WANT IMPLICITLY ENABLED COPY OR MOVE SEMANTICS
            basic_chunk() noexcept                              = delete;
            basic_chunk(const basic_chunk&) noexcept            = delete;
            basic_chunk& operator=(const basic_chunk&) noexcept = delete;
            basic_chunk(basic_chunk&&) noexcept                 = delete;
            basic_chunk& operator=(basic_chunk&&) noexcept      = delete;

            ~basic_chunk() noexcept                             = default;

            // compute the chunk's CRC32 checksum and check whether it is same as the one stored in the chunk
            bool __stdcall is_checksum_valid() const noexcept {
                return checksum == internal::crc::get(name, PNG_CHUNK_NAME_LENGTH + length);
            }

            friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const basic_chunk& chunk) noexcept {
                wostr << L"---------------------------------------\n";
                wostr << L"| Length      " << std::setw(20) << chunk.length << L"|\n";
                wostr << L"| Name        " << std::setw(20) << static_cast<wchar_t>(chunk.name[0]) << static_cast<wchar_t>(chunk.name[1])
                      << static_cast<wchar_t>(chunk.name[2]) << static_cast<wchar_t>(chunk.name[3]) << L"|\n";
                wostr << L"| Data        " << std::setw(20) << std::hex << std::uppercase << chunk.data << L"|\n";
                wostr << std::dec;
                wostr << L"| Checksum    " << std::setw(20) << chunk.checksum << L"|\n";
                wostr << L"---------------------------------------\n";
                return wostr;
            }
    };

    // IHDR, PLTE, IDAT & IEND are critical PNG chunks that must be present in all PNG images
    // IHDR stands for Image HeaDeR, which is the first chunk in a PNG data stream
    class IHDR final : public basic_chunk {
        public:
            explicit IHDR(_In_ const unsigned char* const filebuffer) noexcept : basic_chunk(filebuffer) { }

            bool is_valid() const noexcept {
                return is_checksum_valid() && !length && !data;
                // && ::strcmp(name, "IHDR") won't work here since it expects two null terminated strings
            }
    };

    class PLTE final : public basic_chunk { };

    // IDAT stands for Image DATa
    class IDAT final : public basic_chunk { };

    // image trailer, the last chunk in a PNG data stream
    class IEND final : public basic_chunk { };

} // namespace internal

class png final {
    private:
        [[nodiscard]] bool __stdcall scan_and_parse() noexcept { }

        // critical chunks

    public:
        friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const png& image) noexcept {
            //
            return wostr;
        }
};

#undef __INTERNAL
