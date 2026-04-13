#pragma once

// clang-format off
#include <_internal.hpp>
#include <_bitmap.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>
#include <_utilities.hpp>
#include <_wingdi.hpp>
// clang-format on

#include <vector>

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
    // https://github.com/retorillo/icokit/tree/master

    public:
        // type of the file
        enum ICO_FILE_TYPE : unsigned short { ICON = 1, CURSOR = 2 }; // NOLINT(performance-enum-size) deliberate decision

        // type of the resources contained in the file
        enum ICO_RESOURCE_TYPE : int { BMP, PNG }; // NOLINT(performance-enum-size) needs to be a signed integer

        // look up Raymond Chen's article https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083 for reference.
        // UNFORTUNATELY MICROSOFT DOES NOT INCLUDE A HEADER IN THE WINDOWS SDK THAT DEFINES STRUCTURES ASSOCIATED WITH THE ICO FILE FORMAT
        // EVEN CHROMIUM HAS THESE STRUCTURES HANDROLLED IN ITS SOURCE :(

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

        unsigned char*            buffer;      // the raw file buffer
        unsigned long             file_size;   // file size
        unsigned long             buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse
        ICONDIR                   directory;
        std::vector<ICONDIRENTRY> entries; // metadata of entries stored in the file

        [[nodiscard]] static ICONDIR parse_icondirectory(const unsigned char* const imstream, [[maybe_unused]] const unsigned long long& size) noexcept {
            static_assert(sizeof(ICONDIR) == 24);
            static_assert(std::is_standard_layout_v<ICONDIR>);
            assert(size >= sizeof(ICONDIR));

            ICONDIR temp {};

            if (!imstream) [[unlikely]] {
                ::fprintf(stderr, "Error in %s, received buffer is empty!\n", __PRETTY_FUNCTION__);
                return temp;
            }

            if ((temp.idReserved = *reinterpret_cast<const unsigned short*>(imstream))) [[unlikely]] { // must be 0
                ::fprintf(stderr, "Error in %s, a non-zero value encountered as idReserved!\n", __PRETTY_FUNCTION__);
                return {};
            }

            temp.idType = *reinterpret_cast<const unsigned short*>(imstream + 2);
            if (temp.idType != utilities::to_underlying(ICO_FILE_TYPE::ICON) && temp.idType != utilities::to_underlying(ICO_FILE_TYPE::CURSOR))
                [[unlikely]] { // cannot be anything else
                ::fprintf(stderr, "Error in %s, file is found not to be of type ICON or CURSOR!\n", __PRETTY_FUNCTION__);
                return {};
            }

            // we're 4 bytes past the beginning of the buffer now
            // issue a warning if the file contains more resources than MAX_ALLOWED_ICONDIRENTRIES_PER_FILE
            if ((temp.idCount = *reinterpret_cast<const unsigned short*>(imstream + 4)) > MAX_ALLOWED_ICONDIRENTRIES_PER_FILE) [[unlikely]]
                ::fprintf(stderr, "Warning from %s, file contains more ICONDIRENTRYs than this class can accommodate!\n", __PRETTY_FUNCTION__);
            return temp;
        }

        // it is the caller's responsibility to correctly augment the buffer such that it begins with the binary data of a GRPICONDIRENTRY
        [[nodiscard]] static ICONDIRENTRY parse_icondirectory_entry(
            const unsigned char* const imstream, [[maybe_unused]] const unsigned long long& size
        ) noexcept {
            static_assert(sizeof(ICONDIRENTRY) == 16);
            static_assert(std::is_standard_layout_v<ICONDIRENTRY>);
            assert(size >= sizeof(ICONDIRENTRY));

            ICONDIRENTRY temp {};

            if (!imstream) [[unlikely]] {
                ::fprintf(stderr, "Error in %s, received buffer is empty!\n", __PRETTY_FUNCTION__);
                return temp;
            }

            if ((temp.bReserved = *(imstream + 3))) { // must always be 0
                ::fprintf(stderr, "Error in %s, a non-zero value encountered as bReserved!\n", __PRETTY_FUNCTION__);
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

            return temp;
        }

        [[nodiscard]] static BITMAPINFOHEADER parse_info_header(const unsigned char* const imstream, const size_t& length) noexcept {
            // alignment of wingdi's BITMAPINFOHEADER members makes it inherently packed :)
            static_assert(sizeof(BITMAPINFOHEADER) == 40LLU);
            assert(length >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

            BITMAPINFOHEADER header {};
            if (!imstream) [[unlikely]] {
                ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
                return header;
            }

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

            return header;
        }

    public:
        icon_directory() noexcept = delete;

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init), intentional
        explicit icon_directory(const char* const filename) noexcept :
            buffer { io::imopen(filename, file_size) }, buffer_size { file_size }, directory { icon_directory::parse_icondirectory(buffer, buffer_size) } {
            if (!buffer) [[unlikely]] {
                file_size = 0;
                ::fprintf(stderr, "Error inside %s, object is default initialized as a fallback\n", __PRETTY_FUNCTION__);
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

        icon_directory(const icon_directory& other) noexcept { }

        icon_directory(icon_directory&& other) noexcept { }

        icon_directory& operator=(const icon_directory& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        icon_directory& operator=(icon_directory&& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        ~icon_directory() noexcept {
            delete[] buffer;
            ::memset(this, 0U, sizeof(icon_directory)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        size_type resource_count() const noexcept { return directory.idCount; }

        bool to_file(const char* const filename) const noexcept { return io::imwrite(filename, buffer, buffer_size); }

        bitmap to_bitmap(const unsigned long long& position = 0) const noexcept {
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

        friend std::ostream& operator<<(std::ostream& ostr, const icon_directory& image) noexcept {
            ostr << "---------------------------------------\n";
            ostr << "| idReserved      " << std::setw(20) << image.directory.idReserved << "|\n";
            ostr << "| idType          " << std::setw(20) << image.directory.idType << "|\n";
            ostr << "| idCount         " << std::setw(20) << image.directory.idCount << "|\n";
            ostr << "---------------------------------------\n";
            for (const auto& entry : image.entries) {
                ostr << "| bWidth          " << std::setw(20) << entry.bWidth << "|\n";
                ostr << "| bHeight         " << std::setw(20) << entry.bHeight << "|\n";
                ostr << "| bColorCount     " << std::setw(20) << entry.bColorCount << "|\n";
                ostr << "| bReserved       " << std::setw(20) << entry.bReserved << "|\n";
                ostr << "| wPlanes         " << std::setw(20) << entry.wPlanes << "|\n";
                ostr << "| wBitCount       " << std::setw(20) << entry.wBitCount << "|\n";
                ostr << "| dwBytesInRes    " << std::setw(20) << entry.dwBytesInRes << "|\n";
                ostr << "| dwImageOffset   " << std::setw(20) << entry.dwImageOffset << "|\n";
                ostr << "=======================================\n";
            }
            ostr << "---------------------------------------\n";
            return ostr;
        }
};

// static_assert(std::is_standard_layout_v<icon_directory>); // well, damn
#undef __INTERNAL
