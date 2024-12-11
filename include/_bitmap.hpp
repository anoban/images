#pragma once
#define __INTERNAL
#if !defined(__BITMAP) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#define _AMD64_ // architecture
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <type_traits>

// project headers
#include <_helpers.hpp>
#include <_imageio.hpp>
#include <_iterators.hpp>

// Windows .bmp format supports 1, 4, 8, 16, 24 and 32 bits pixel depths
// eventhough Windows bitmaps support simple run length compression for pixels with 4 or 8 bits depth, it's rarely used as this compression gives tangible
// reductions only with large pixel blocks of identical colours, which is rare in real life images
// multibyte integers in bitmaps are stored LSB first (using little endian byte order)

// since bitmaps originted in Microsoft, Windows SDK (wingdi.h) comes pre-packed with almost all the necessary data structures and routines to handle bitmaps

// every Windows bitmap file begins with a BITMAPFILEHEADER struct, this helps in recognizing the file as a bitmap, the first two bytes will be 'B', 'M'
// image header comes in two variants, one representing OS/2 bitmap format (BITMAPCOREHEADER) and the other representing the most common Windows bitmap format (BITMAPINFOHEADER)
// however there are no markers to identify the type of the image header present in the bmp image, the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct
// from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes

class bitmap { // this class is designed to represent what Windows calls as DIBs (Device Independent Bitmap)
    public:
        using value_type      = RGBQUAD; // pixel type
        using pointer         = value_type*;
        using const_pointer   = const value_type*;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using iterator        = ::random_access_iterator<value_type>;
        using const_iterator  = ::random_access_iterator<const value_type>;
        using size_type       = long long; // because BMP files use signed long as the type for sizes
        using difference_type = long long;

        // order of pixels in the BMP buffer.
        enum class SCANLINE_ORDERING : unsigned char { TOPDOWN, BOTTOMUP };

        // types of compressions used in BMP files.
        enum class COMPRESSION_KIND : unsigned char { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN };

        // that's 'M' followed by a 'B' (LE), wingdi's BITMAPFILEHEADER uses a  unsigned short for SOI instead of two chars
        static constexpr unsigned short SOI { 'B' | 'M' << 8 }; // Start Of Image

        // clang-format off
#ifndef __TEST__
   protected:
#endif
        // clang-format on

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        unsigned char*                  buffer; // this will point to the original file buffer, this is the one that needs deallocation!
        RGBQUAD*                        pixels; // this points to the start of pixels in the file buffer i.e offset buffer + 54
        BITMAPFILEHEADER                file_header;
        BITMAPINFOHEADER                info_header;
        unsigned long                   file_size; // file size
        unsigned long buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse
                                   // NOLINTEND(misc-non-private-member-variables-in-classes)

        friend class icondirectory; // to serialize ico objects as bitmaps, we need access to this class's internals

        // using a new buffer to store pixels sounds like a clean design but it brings a string of performance issues
        // 1) entails an additional heap allocation (pixel buffer) and a deallocation (the raw file buffer)
        // 2) now that the buffer only holds pixels, we'll need to serialize the metadata structs separately when serializing the image

        // this can be accomplished using two approaches ::
        // 1) by constructing a temporary array of 54 bytes that hold these structs, and calling CreateFileW with CREATE_NEW first to serialize the
        // structs, closing that file and then reopening the serialized file using CreateFileW with OPEN_EXISTING and FILE_APPEND_DATA flags to append the pixel buffer
        // (paying the penalty for two syscalls)
        // 2) we could create a new buffer (gratuitous heap allocation) with enough space for the structs and pixels and then copy the structs and pixels there first,
        // followed by serialization of this new buffer with a call to CreateFileW

        // the best (admittedly less elegant) way to circumnavigate this is to hold an alias pointer that points to the beginning of the pixel buffer
        // inside the raw file buffer - no additional heap allocations and we could just sink the buffer to the disk when a serialization is needed

        [[nodiscard]] static BITMAPFILEHEADER __stdcall parse_file_header(
            _In_reads_bytes_(length) const unsigned char* const imstream, _In_ const size_t& length
        ) noexcept {
            static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "struct BITMAPFILEHEADER must be 14 bytes in size");
            assert(length >= sizeof(BITMAPFILEHEADER));
            BITMAPFILEHEADER header {};
            if (!imstream) return header;

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (imstream[0] != 'B' && imstream[1] != 'M') [[unlikely]] { // validate that the passed buffer is of a bitmap file
                ::fputws(L"Error in " __FUNCTIONW__ ", file isn't recognized as a Windows bitmap file\n", stderr);
                return header;
            }

            header.bfType      = SOI; // 'B', 'M'
                                      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            header.bfSize      = *reinterpret_cast<const unsigned int*>(imstream + 2); // file size in bytes
            header.bfReserved1 = header.bfReserved2 = 0;                               // 4 reserved bytes
            // offset from the beginning of BITMAPFILEHEADER struct to the start of pixel data
            header.bfOffBits = *reinterpret_cast<const unsigned int*>( // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                imstream + 10
            );
            return header;
        }

        // find the type of compression used in the BMP file, bitmap files, in general aren't compressed
        [[nodiscard]] static constexpr COMPRESSION_KIND __stdcall get_compression_kind(_In_ const unsigned kind) noexcept {
            switch (kind) {
                case 0  : return COMPRESSION_KIND::RGB; // uncompressed RGBQUAD pixels
                case 1  : return COMPRESSION_KIND::RLE8;
                case 2  : return COMPRESSION_KIND::RLE4;
                case 3  : return COMPRESSION_KIND::BITFIELDS;
                default : break;
            }
            return COMPRESSION_KIND::UNKNOWN;
        }

        [[nodiscard]] static BITMAPINFOHEADER __stdcall parse_info_header(
            _In_reads_bytes_(length) const unsigned char* const imstream, _In_ const size_t& length
        ) noexcept {
            // alignment of wingdi's BITMAPINFOHEADER members makes it inherently packed :)
            static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "struct BITMAPINFOHEADER must be 40 bytes in size");
            assert(length >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));
            BITMAPINFOHEADER header {};
            if (!imstream) return header;

            if (*reinterpret_cast<const unsigned int*>(imstream + 14U) != 40) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                [[unlikely]] {
                // header size must be == 40 bytes
                ::fputws(L"Error in " __FUNCTIONW__ ": unparseable BITMAPINFOHEADER\n", stderr);
                return header;
            }

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            header.biSize          = *reinterpret_cast<const unsigned int*>(imstream + 14); // size of the BITMAPINFOHEADER struct
            header.biWidth         = *reinterpret_cast<const int*>(imstream + 18);          // width of the bitmap image in pixels
            header.biHeight        = *reinterpret_cast<const int*>(imstream + 22);          // height of the bitmap image in pixels
                // bitmaps with a negative height may not be compressed
            header.biPlanes        = *reinterpret_cast<const unsigned short*>(imstream + 26); // must be 1
            header.biBitCount      = *reinterpret_cast<const unsigned short*>(imstream + 28); // 1, 4, 8, 16, 24 or 32
            header.biCompression   = static_cast<decltype(BITMAPINFOHEADER::biCompression)>(  // compression kind (if compressed)
                bitmap::get_compression_kind(*reinterpret_cast<const unsigned int*>(imstream + 30U))
            );
            header.biSizeImage     = *reinterpret_cast<const unsigned int*>(imstream + 34); // 0 if not compressed
            header.biXPelsPerMeter = *reinterpret_cast<const int*>(imstream + 38);    // resolution in pixels per meter along the x axis
            header.biYPelsPerMeter = *reinterpret_cast<const int*>(imstream + 42);    // resolution in pixels per meter along the y axis
            header.biClrUsed = *reinterpret_cast<const unsigned int*>(imstream + 46); // number of entries in the colourmap that are used
            header.biClrImportant = *reinterpret_cast<const unsigned int*>(imstream + 50); // number of important colors
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            return header;
        }

        [[nodiscard]] static constexpr SCANLINE_ORDERING __stdcall get_scanline_order(_In_ const BITMAPINFOHEADER& header) noexcept {
            // BITMAPINFOHEADER::biHeight is usually an unsigned value, a negative value indicates that the scanlines are ordered top down instead of the customary bottom up order
            return header.biHeight >= 0 ? SCANLINE_ORDERING::BOTTOMUP : SCANLINE_ORDERING::TOPDOWN;
        }

    public:
        bitmap() noexcept = default; // that's good enough

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init) - intentional
        explicit bitmap(_In_ const wchar_t* const filename) noexcept : // construct a bitmap from a file on disk
            buffer { internal::open(filename, file_size) },            // a little unorthodox but okay lol
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)}
            pixels { reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) },
            file_header { bitmap::parse_file_header(buffer, file_size) },
            info_header { bitmap::parse_info_header(buffer, file_size) },
            // file_size {}, we want to preserve size's previous state materialized by internal::open()
            buffer_size { file_size } {
            if (!buffer) [[unlikely]] {             // if open failed
                ::memset(this, 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
            }
        }

        // construct a bitmap from a stream of bytes
        bitmap(
            _In_reads_bytes_(_size)
                const unsigned char* const imstream, // this buffer is expected to contain the metadata preceding bitmap pixel buffer too
            _In_ const unsigned long       size
        ) noexcept :
            buffer { new (std::nothrow) unsigned char[size] }, // cannot take ownership of the incoming buffer, so let's copy it
                                                               // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)}
            pixels { reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) },
            // if _buffer is nullptr, the __parse_xxx family of helpers will default initialize the cognate header structs
            file_header { bitmap::parse_file_header(imstream, size) },
            info_header { bitmap::parse_info_header(imstream, size) },
            file_size { size },
            buffer_size { size } {
            if (!buffer) [[unlikely]] {             // has the allocation failed
                ::memset(this, 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            ::memcpy_s(buffer, file_size, imstream, size);
        }

    protected:
        // an aggregate initializer style private constructor
        bitmap(
            _In_ unsigned char* const    imstream,
            _In_ const BITMAPFILEHEADER& file,
            _In_ const BITMAPINFOHEADER& info,
            _In_ const unsigned          size
        ) noexcept :
            buffer { imstream },
            pixels { reinterpret_cast<RGBQUAD*>(imstream) },
            file_header { file },
            info_header { info },
            file_size { size },
            buffer_size { size } { }

        // this is likely to get more compilcated
        bitmap(_In_ const long height, _In_ const long width) noexcept :
            buffer { new (std::nothrow
            ) unsigned char[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * sizeof(RGBQUAD)] },
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pixels { reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) },
            file_header { .bfType = 0x4D42,
                          .bfSize = static_cast<unsigned long>(
                              sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * sizeof(RGBQUAD)
                          ),
                          .bfReserved1 = 0,
                          .bfReserved2 = 0,
                          .bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) },
            info_header { .biSize          = sizeof(BITMAPINFOHEADER),
                          .biWidth         = width,
                          .biHeight        = height,
                          .biPlanes        = 1,
                          .biBitCount      = 32,
                          .biCompression   = BI_RGB, // uncompressed RGB
                          .biSizeImage     = 0,
                          .biXPelsPerMeter = 3780, // an arbitrary choice
                          .biYPelsPerMeter = 3780, // an arbitrary choice
                          .biClrUsed       = 0,
                          .biClrImportant  = 0 },
            file_size { file_header.bfSize },
            buffer_size { file_header.bfSize } {
            if (!buffer) [[unlikely]] {
                ::memset(this, 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            ::memcpy_s(buffer, buffer_size, &file_header, sizeof(BITMAPFILEHEADER));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ::memcpy_s(buffer + sizeof(BITMAPFILEHEADER), buffer_size - sizeof(BITMAPFILEHEADER), &info_header, sizeof(BITMAPINFOHEADER));
        }

    public:
        // copy constructor
        bitmap(_In_ const bitmap& other) noexcept :
            buffer { new (std::nothrow
            ) unsigned char[other.file_size] }, // don't copy the trailing unused bytes from the bitmap, if present

            pixels { // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                     reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
            },
            file_header { other.file_header },
            info_header { other.info_header },
            file_size { other.file_size },
            buffer_size { other.file_size /* not buffer_size */ } {
            if (!buffer) [[unlikely]] {
                ::memset(this, 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            ::memcpy_s(buffer, file_size, other.buffer, other.file_size);
        }

        // copy assignment operator
        bitmap& operator=(_In_ const bitmap& other) noexcept {
            if (std::addressof(other) == this) [[unlikely]]
                return *this;

            // evaluate whether a reallocation is due
            if (buffer_size < other.file_size) { // if the existing buffer cannot hold the incoming object
                delete[] buffer;
                buffer = new (std::nothrow) unsigned char[other.file_size];
                if (!buffer) [[unlikely]] {             // has the allocation failed
                    ::memset(this, 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
                    ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                    return *this;
                }

                file_size = buffer_size = other.file_size; // update the sizes after reallocation
            } else                                         // if no reallocation was needed
                file_size = other.file_size;               // keep the existing buffer_size as we are reusing the existing buffer

            ::memcpy_s(buffer, file_size, other.buffer, other.file_size);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pixels      = reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
            file_header = other.file_header;
            info_header = other.info_header;

            return *this;
        }

        // move constructor
        bitmap(_Inout_ bitmap&& other) noexcept :
            buffer { other.buffer },
            pixels { other.pixels },
            file_header { other.file_header },
            info_header { other.info_header },
            file_size { other.file_size },
            buffer_size { other.buffer_size } {
            ::memset(std::addressof(other), 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        // move assignment operator
        bitmap& operator=(_Inout_ bitmap&& other) noexcept {
            if (std::addressof(other) == this) [[unlikely]]
                return *this;

            delete[] buffer; // give up the old buffer

            buffer      = other.buffer;
            pixels      = other.pixels;
            file_header = other.file_header;
            info_header = other.info_header;
            file_size   = other.file_size;
            buffer_size = other.buffer_size;
            ::memset(std::addressof(other), 0U, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)

            return *this;
        }

        ~bitmap() noexcept {
            delete[] buffer;
            ::memset(this, 0, sizeof(bitmap)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        friend std::wostream& operator<<(_Inout_ std::wostream& wostr, _In_ const bitmap& image) noexcept {
            wostr << L"---------------------------------------\n";
            wostr << L"| bfType          " << std::setw(20) << image.file_header.bfType << L"|\n";
            wostr << L"| bfSize          " << std::setw(20) << image.file_header.bfSize << L"|\n";
            wostr << L"| bfOffBits       " << std::setw(20) << image.file_header.bfOffBits << L"|\n";
            wostr << L"|-------------------------------------|\n";
            wostr << L"| biSize          " << std::setw(20) << image.info_header.biSize << L"|\n";
            wostr << L"| biWidth         " << std::setw(20) << image.info_header.biWidth << L"|\n";
            wostr << L"| biHeight        " << std::setw(20) << image.info_header.biHeight << L"|\n";
            wostr << L"| biPlanes        " << std::setw(20) << image.info_header.biPlanes << L"|\n";
            wostr << L"| biBitCount      " << std::setw(20) << image.info_header.biBitCount << L"|\n";
            wostr << L"| biCompression   " << std::setw(20) << image.info_header.biCompression << L"|\n";
            wostr << L"| biSizeImage     " << std::setw(20) << image.info_header.biSizeImage << L"|\n";
            wostr << L"| biXPelsPerMeter " << std::setw(20) << image.info_header.biXPelsPerMeter << L"|\n";
            wostr << L"| biYPelsPerMeter " << std::setw(20) << image.info_header.biYPelsPerMeter << L"|\n";
            wostr << L"| biClrUsed       " << std::setw(20) << image.info_header.biClrUsed << L"|\n";
            wostr << L"| biClrImportant  " << std::setw(20) << image.info_header.biClrImportant << L"|\n";
            wostr << L"---------------------------------------\n";
            return wostr;
        }

        bool     to_file(_In_ const wchar_t* const filename) const noexcept { return internal::serialize(filename, buffer, file_size); }

        iterator begin() noexcept { // NOLINT(readability-make-member-function-const)
            return { pixels, static_cast<iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        const_iterator begin() const noexcept {
            return { pixels, static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        const_iterator cbegin() const noexcept {
            return { pixels, static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        iterator end() noexcept { // NOLINT(readability-make-member-function-const)
            return { pixels,
                     static_cast<iterator::size_type>(info_header.biHeight * info_header.biWidth),
                     static_cast<iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        const_iterator end() const noexcept {
            return { pixels,
                     static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth),
                     static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        const_iterator cend() const noexcept {
            return { pixels,
                     static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth),
                     static_cast<const_iterator::size_type>(info_header.biHeight * info_header.biWidth) };
        }

        reference operator[](_In_ const size_type offset) noexcept { // NOLINT(readability-make-member-function-const)
            assert(offset < info_header.biHeight * info_header.biWidth);
            return pixels[offset]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        const_reference operator[](_In_ const size_type offset) const noexcept {
            assert(offset < info_header.biHeight * info_header.biWidth);
            return pixels[offset]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        pointer                         data() noexcept { return pixels; } // NOLINT(readability-make-member-function-const)

        const_pointer                   data() const noexcept { return pixels; }

        // returns the height and width of the bitmap, in that order
        std::pair<size_type, size_type> dim() const noexcept { return { info_header.biHeight, info_header.biWidth }; }

        long                            height() const noexcept { return info_header.biHeight; }

        long                            width() const noexcept { return info_header.biWidth; }
};

#undef __INTERNAL
