#pragma once
#define __INTERNAL
#if !defined(__ICO) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <vector>

#include <_bitmap.hpp>
#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>

// most .ico files will have only one bitmap in them, so this is generous enough
static constexpr unsigned long long MAX_ALLOWED_ICONDIRENTRIES_PER_FILE { 0x01 << 6 };

// an ICO file can be imagined as a meta-info struct, called ICONDIR, for ICON DIRectory followed by a bitmap or an array of bitmaps
// (an .ico file can contain one or more images, hence the name icon directory), these bitmap images are stored contiguously, following the ICONDIR structure.
// each bitmap is defined by an GRPICONDIRENTRY struct in the ICONDIR struct
// the bitmap data can be in the format of a Windows BMP without the BITMAPFILEHEADER struct or a PNG image in its entirety i.e uncompressed.
// https://handwiki.org/wiki/ICO_(file_format)

// multibyte integers in .ico files are stored LSB first (using little endian byte order), just like .bmp files

// in summary, the binary representation of an .ico file looks like { GRPICONDIRENTRY, pixels, <GRPICONDIRENTRY, pixels> ... }

class icondirectory                 final { // represents an .ico file

    public:
        // type of the file
        enum ICO_FILE_TYPE : unsigned short { ICON = 1, CURSOR = 2 }; // NOLINT(performance-enum-size) deliberate decision

        // type of the resources contained in the file
        enum ICO_RESOURCE_TYPE : int { BMP, PNG }; // NOLINT(performance-enum-size) needs to be a signed integer

        // look up Raymond Chen's article https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083 for reference.
        // UNFORTUNATELY MICROSOFT DOES NOT INCLUDE A HEADER IN THE WINDOWS SDK THAT DEFINES STRUCTURES ASSOCIATED WITH THE ICO FILE FORMAT
        // EVEN CHROMIUM HAS THESE STRUCTURES HANDROLLED IN ITS SOURCE :(

        struct ICONDIRENTRY final {
                BYTE  bWidth;  // width of the associated image in pixels (must be in the range of 0 to 256)
                BYTE  bHeight; // height of the associated image in pixels (must be in the range of 0 to 256)
                BYTE  bColorCount; // number of colours in the colur palette, must be 0 if the image doesn't use a colour palette
                BYTE  bReserved; // reserved byte, must always be 0
                WORD  wPlanes;   // for icons- specifies the colour planes (should be 0 or 1)
                // for cursors - specifies the horizontal coordinate of the hotspot as offset from the left, in pixels
                WORD  wBitCount; // for icons - specifies pixel depth
                // for cursors - specifies the vertical coordinate of the hotspot as offset from the top, in pixels
                // Windows cursors have a hotspot location that decides one exact point that is affected by mouse events https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors
                DWORD dwBytesInRes;  // size of the associated image in bytes
                DWORD dwImageOffset; // offset of the associated image data, from the beginning of the .ico or .cur file
        };

        struct ICONDIR final {
                WORD          idReserved; // reserved, must always be 0
                WORD          idType; // specifies the type of the resources contained, values other than 1 and 2 are invalid
                    // an ICONDIR can store one or more of either icon or cursor type resources, heterogeneous mixtures of icons and cursors aren't permitted
                WORD          idCount; // number of resources (images) stored in the given .ico file
                ICONDIRENTRY* idEntries;
        };

        struct ICONIMAGE final {
                //
        };

        using value_type      = RGBQUAD; // pixel type
        using pointer         = value_type*;
        using const_pointer   = const value_type*;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using iterator        = ::random_access_iterator<value_type>;
        using const_iterator  = ::random_access_iterator<const value_type>;
        using size_type       = unsigned long long;
        using difference_type = long long;

        // clang-format off
#ifndef __TEST__
    private:
#endif
        // clang-format on

        unsigned char* buffer;    // the raw byte buffer
        unsigned long  file_size; // file size
        unsigned long  buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse
        ICONDIR        directory;
        std::vector<ICONDIRENTRY> entries; // metadata of entries stored in the file

        [[nodiscard]] static ICONDIR __stdcall parse_icondirectory(
            _In_reads_bytes_(size) const unsigned char* const imstream, _In_ [[maybe_unused]] const unsigned long long& size
        ) noexcept {
            static_assert(sizeof(ICONDIR) == 16);
            static_assert(std::is_standard_layout_v<ICONDIR>);
            assert(size >= sizeof(ICONDIR));

            ICONDIR temp {};

            if (!imstream) [[unlikely]] {
                ::fputws(L"Error in " __FUNCTIONW__ ", received buffer is empty!\n", stderr);
                return temp;
            }

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)
            if ((temp.idReserved = *reinterpret_cast<const unsigned short*>(imstream))) [[unlikely]] { // must be 0
                ::fputws(L"Error in " __FUNCTIONW__ ", a non-zero value encountered as idReserved!\n", stderr);
                return {};
            }

            temp.idType = *reinterpret_cast<const unsigned short*>(imstream + 2);
            if (temp.idType != ICO_FILE_TYPE::ICON && temp.idType != ICO_FILE_TYPE::CURSOR) [[unlikely]] { // cannot be anything else
                ::fputws(L"Error in " __FUNCTIONW__ ", file is found not to be of type ICON or CURSOR!\n", stderr);
                return {};
            }

            // we're 4 bytes past the beginning of the buffer now
            // issue a warning if the file contains more resources than MAX_ALLOWED_ICONDIRENTRIES_PER_FILE
            if ((temp.idCount = *reinterpret_cast<const unsigned short*>(imstream + 4)) > MAX_ALLOWED_ICONDIRENTRIES_PER_FILE) [[unlikely]]
                ::fputws(L"Warning from " __FUNCTIONW__ ", file contains more ICONDIRENTRYs than this class can accommodate!\n", stderr);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)
            return temp;
        }

        // it is the caller's responsibility to correctly augment the buffer such that it begins with the binary data of a GRPICONDIRENTRY
        [[nodiscard]] static ICONDIRENTRY __stdcall parse_icondirectory_entry(
            _In_count_(size) const unsigned char* const imstream, _In_ [[maybe_unused]] const unsigned long long& size
        ) noexcept {
            static_assert(sizeof(ICONDIRENTRY) == 16);
            static_assert(std::is_standard_layout_v<ICONDIRENTRY>);
            assert(size >= sizeof(ICONDIRENTRY));

            ICONDIRENTRY temp {};

            if (!imstream) [[unlikely]] {
                ::fputws(L"Error in " __FUNCTIONW__ ", received buffer is empty!\n", stderr);
                return temp;
            }

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)
            if ((temp.bReserved = *(imstream + 3))) { // must always be 0
                ::fputws(L"Error in " __FUNCTIONW__ ", a non-zero value encountered as bReserved!\n", stderr);
                return {};
            }

            temp.bWidth        = *imstream;
            temp.bHeight       = *(imstream + 1);
            temp.bColorCount   = *(imstream + 2);
            // temp.bReserved     = *(imstream + 3);
            temp.wPlanes       = *reinterpret_cast<const unsigned short*>(imstream + 4);
            temp.wBitCount     = *reinterpret_cast<const unsigned short*>(imstream + 6);
            temp.dwBytesInRes  = *reinterpret_cast<const unsigned long*>(imstream + 8);
            temp.dwImageOffset = *reinterpret_cast<const unsigned long*>(imstream + 12);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)

            return temp;
        }

    public:
        icondirectory() noexcept = default; // will be good enough

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init), intentional
        explicit icondirectory(_In_ const wchar_t* const filename) noexcept :
            buffer { internal::open(filename, file_size) },
            buffer_size { file_size },
            directory { icondirectory::parse_icondirectory(buffer, buffer_size) } {
            if (!buffer) [[unlikely]] {
                file_size = 0;
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            // if any errors encountered in the reading and parsing the functions responsible for those taks will take care of the error reporting

            entries.resize(directory.idCount);
            unsigned long long caret { 6 }; // skip the first six bytes and jump to the first GRPICONDIRENTRY
            for (unsigned i = 0; i < directory.idCount; ++i) { // try and parse all the ICONDIRENTRYs in the file
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                entries.at(i)  = icondirectory::parse_icondirectory_entry(buffer + caret, file_size);
                // update caret
                caret         += sizeof(ICONDIRENTRY);
            }
        }

        icondirectory(_In_ const icondirectory& other) noexcept { }

        icondirectory(_In_ icondirectory&& other) noexcept { }

        icondirectory& operator=(_In_ const icondirectory& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        icondirectory& operator=(_In_ icondirectory&& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        ~icondirectory() noexcept {
            delete[] buffer;
            ::memset(this, 0U, sizeof(icondirectory)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        size_type resource_count() const noexcept { return directory.idCount; }

        bool      to_file(_In_ const wchar_t* const filename) const noexcept { return internal::serialize(filename, buffer, buffer_size); }

        bitmap    to_bitmap(_In_opt_ const unsigned long long& position = 0) const noexcept {
            //
            if (position >= directory.idCount) {
                //
                return {};
            }

            bitmap image { entries.at(position).bHeight, entries.at(position).bWidth };
            // ::memcpy_s(image.pixels, image.buffer_size, buffer + entries.at(position).dwImageOffset, entries.at(position).dwBytesInRes);

            return image;
        }

        friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const icondirectory& image) noexcept {
            wostr << L"---------------------------------------\n";
            wostr << L"| idReserved      " << std::setw(20) << image.directory.idReserved << L"|\n";
            wostr << L"| idType          " << std::setw(20) << image.directory.idType << L"|\n";
            wostr << L"| idCount         " << std::setw(20) << image.directory.idCount << L"|\n";
            wostr << L"---------------------------------------\n";
            for (const auto& entry : image.entries) {
                wostr << L"| bWidth          " << std::setw(20) << entry.bWidth << L"|\n";
                wostr << L"| bHeight         " << std::setw(20) << entry.bHeight << L"|\n";
                wostr << L"| bColorCount     " << std::setw(20) << entry.bColorCount << L"|\n";
                wostr << L"| bReserved       " << std::setw(20) << entry.bReserved << L"|\n";
                wostr << L"| wPlanes         " << std::setw(20) << entry.wPlanes << L"|\n";
                wostr << L"| wBitCount       " << std::setw(20) << entry.wBitCount << L"|\n";
                wostr << L"| dwBytesInRes    " << std::setw(20) << entry.dwBytesInRes << L"|\n";
                wostr << L"| dwImageOffset   " << std::setw(20) << entry.dwImageOffset << L"|\n";
                wostr << L"=======================================\n";
            }
            wostr << L"---------------------------------------\n";
            return wostr;
        }
};

#undef __INTERNAL
