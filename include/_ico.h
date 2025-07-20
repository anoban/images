#pragma once

// clang-format off
#include <internal.hpp>
// clang-format on

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

class icon_directory final { // represents an .ico file

    public:
        // type of the file
        enum class ICO_FILE_TYPE : unsigned short { ICON = 1, CURSOR = 2 }; // NOLINT(performance-enum-size) deliberate decision

        // type of the resources contained in the file
        enum class ICO_RESOURCE_TYPE : int { BMP, PNG }; // NOLINT(performance-enum-size) needs to be a signed integer

        // look up Raymond Chen's article https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083 for reference.
        // UNFORTUNATELY MICROSOFT DOES NOT INCLUDE A HEADER IN THE WINDOWS SDK THAT DEFINES STRUCTURES ASSOCIATED WITH THE ICO FILE FORMAT
        // EVEN CHROMIUM HAS THESE STRUCTURES HANDROLLED IN ITS SOURCE :(

        struct ICONDIRENTRY final {
                BYTE  bWidth;      // width of the associated image in pixels (must be in the range of 0 to 256)
                BYTE  bHeight;     // height of the associated image in pixels (must be in the range of 0 to 256)
                BYTE  bColorCount; // number of colours in the colur palette, must be 0 if the image doesn't use a colour palette
                BYTE  bReserved;   // reserved byte, must always be 0
                WORD  wPlanes;     // for .ico - specifies the colour planes (should be 0 or 1)
                // for .cur - specifies the horizontal coordinate of the hotspot as offset from the left, in pixels
                WORD  wBitCount; // for .ico - specifies pixel depth
                // for .cur - specifies the vertical coordinate of the hotspot as offset from the top, in pixels
                // Windows cursors have a hotspot location that decides one exact point that is affected by mouse events https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors
                DWORD dwBytesInRes;  // size of the associated image in bytes
                DWORD dwImageOffset; // offset of the associated image data, from the beginning of the .ico or .cur file
        };

        struct ICONDIR final {
                WORD         idReserved; // reserved, must always be 0
                WORD         idType;     // specifies the type of the resources contained, values other than 1 and 2 are invalid
                // an ICONDIR can store one or more of either icon or cursor type resources, heterogeneous mixtures of icons and cursors aren't permitted
                WORD         idCount;      // number of resources (images) stored in the given .ico file
                ICONDIRENTRY idEntries[1]; // NOLINT(modernize-avoid-c-arrays)
        };

        struct ICONIMAGE final {
                BITMAPINFOHEADER icHeader;
                RGBQUAD          icColors[1]; // NOLINT(modernize-avoid-c-arrays)
                BYTE             icXOR[1];    // NOLINT(modernize-avoid-c-arrays)
                BYTE             icAND[1];    // NOLINT(modernize-avoid-c-arrays)
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

        unsigned char* buffer;      // the raw file buffer
        unsigned long  file_size;   // file size
        unsigned long  buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse
        ICONDIR        directory;
        std::vector<ICONDIRENTRY> entries; // metadata of entries stored in the file

        [[nodiscard]] static ICONDIR __stdcall parse_icondirectory(
            _In_reads_bytes_(size) const unsigned char* const imstream, _In_ [[maybe_unused]] const unsigned long long& size
        ) noexcept {
            static_assert(sizeof(ICONDIR) == 24);
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
            if (temp.idType != internal::to_underlying(ICO_FILE_TYPE::ICON) &&
                temp.idType != internal::to_underlying(ICO_FILE_TYPE::CURSOR)) [[unlikely]] { // cannot be anything else
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

        [[nodiscard]] static BITMAPINFOHEADER __stdcall parse_info_header(
            _In_reads_bytes_(length) const unsigned char* const imstream, _In_ const size_t& length
        ) noexcept {
            // alignment of wingdi's BITMAPINFOHEADER members makes it inherently packed :)
            static_assert(sizeof(BITMAPINFOHEADER) == 40LLU);
            assert(length >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

            BITMAPINFOHEADER header {};
            if (!imstream) [[unlikely]] {
                ::fputws(L"Error in " __FUNCTIONW__ ", received an empty buffer\n", stderr);
                return header;
            }

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)
            // if ((header.biSize = *reinterpret_cast<const unsigned*>(imstream + 14) != 40)) [[unlikely]] {
            //     // size of the BITMAPINFOHEADER struct must be == 40 bytes
            //     ::fputws(L"Error in " __FUNCTIONW__ ": unparseable BITMAPINFOHEADER\n", stderr);
            //     return {}; // DO NOT RETURN THE PLACEHOLDER BECAUSE IT WOULD HAVE POTENTIALLY BEEN INCORRECTLY UPDATED AT THIS POINT
            // }

            header.biSize          = *reinterpret_cast<const unsigned*>(imstream + 14);
            header.biWidth         = *reinterpret_cast<const int*>(imstream + 18); // width of the bitmap image in pixels
            header.biHeight        = *reinterpret_cast<const int*>(imstream + 22); // height of the bitmap image in pixels
            // bitmaps with a negative height may not be compressed
            header.biPlanes        = *reinterpret_cast<const unsigned short*>(imstream + 26); // must be 1
            header.biBitCount      = *reinterpret_cast<const unsigned short*>(imstream + 28); // 1, 4, 8, 16, 24 or 32
            header.biCompression   = static_cast<decltype(BITMAPINFOHEADER::biCompression)>(  // compression kind (if compressed)
                bitmap::get_compression_kind(*reinterpret_cast<const unsigned*>(imstream + 30U))
            );
            header.biSizeImage     = *reinterpret_cast<const unsigned*>(imstream + 34); // 0 if not compressed
            header.biXPelsPerMeter = *reinterpret_cast<const int*>(imstream + 38);      // resolution in pixels per meter along the x axis
            header.biYPelsPerMeter = *reinterpret_cast<const int*>(imstream + 42);      // resolution in pixels per meter along the y axis
            header.biClrUsed       = *reinterpret_cast<const unsigned*>(imstream + 46); // number of entries in the colourmap that are used
            header.biClrImportant  = *reinterpret_cast<const unsigned*>(imstream + 50); // number of important colors
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-assignment-in-if-condition)

            return header;
        }

    public:
        icon_directory() noexcept = delete;

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init), intentional
        explicit icon_directory(_In_ const wchar_t* const filename) noexcept :
            buffer { internal::open(filename, file_size) },
            buffer_size { file_size },
            directory { icon_directory::parse_icondirectory(buffer, buffer_size) } {
            if (!buffer) [[unlikely]] {
                file_size = 0;
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            // if any errors encountered in the reading and parsing the functions responsible for those taks will take care of the error reporting

            entries.resize(directory.idCount);
            unsigned long long caret { 6 };                    // skip the first six bytes and jump to the first ICONDIRENTRY structure
            for (unsigned i = 0; i < directory.idCount; ++i) { // try and parse all the ICONDIRENTRYs in the file
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                entries.at(i)  = icon_directory::parse_icondirectory_entry(buffer + caret, file_size);
                // update caret
                caret         += sizeof(ICONDIRENTRY);
            }
        }

        icon_directory(_In_ const icon_directory& other) noexcept { }

        icon_directory(_In_ icon_directory&& other) noexcept { }

        icon_directory& operator=(_In_ const icon_directory& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        icon_directory& operator=(_In_ icon_directory&& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        ~icon_directory() noexcept {
            delete[] buffer;
            ::memset(this, 0U, sizeof(icon_directory)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        size_type resource_count() const noexcept { return directory.idCount; }

        bool to_file(_In_ const wchar_t* const filename) const noexcept { return internal::serialize(filename, buffer, buffer_size); }

        bitmap to_bitmap(_In_opt_ const unsigned long long& position = 0) const noexcept {
            //
            if (position >= directory.idCount) {
                //
                return {};
            }

            // allocate the memory needed to extract the selected icon aa a bitmap object
            bitmap image { entries.at(position).bHeight, entries.at(position).bWidth };
            // !!! TODO: INCORRECT!!! DOESN'T COPY ALL THE BYTES FROM THE ICON RESOURCE BECAUSE FOR SOME REASON WIDTH * HEIGHT * sizeof(RGBQUAD) ends up smaller than dwBytesInRes
            ::memcpy_s(
                image.pixels,
                image.buffer_size,
                buffer + entries.at(position).dwImageOffset,
                /* entries.at(position).dwBytesInRes */ image.buffer_size - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER)
            );
            // image.info_header = icon_directory::parse_info_header(buffer + entries.at(position).dwImageOffset, buffer_size);

            return image;
        }

        friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const icon_directory& image) noexcept {
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

// static_assert(std::is_standard_layout_v<icon_directory>); // well, damn
#undef __INTERNAL
