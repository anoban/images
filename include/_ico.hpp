#pragma once
#define __INTERNAL
#if !defined(__ICO) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <vector>

#include <_bitmap.hpp>
#include <_helpers.hpp>
#include <_imageio.hpp>

// most .ico files will have only one bitmap in them, so this is generous enough
static constexpr unsigned long long MAX_ALLOWED_ICONDIRENTRIES_PER_FILE { 0x01 << 5 };

// an ICO file can be imagined as a meta-info struct, called ICONDIR, for ICON DIRectory followed by a bitmap or an array of bitmaps
// (an .ico file can contain one or more images, hence the name icon directory), these bitmap images are stored contiguously, following the ICONDIR structure.
// each bitmap is defined by an ICONDIRENTRY struct in the ICONDIR struct
// the bitmap data can be in the format of a Windows BMP without the BITMAPFILEHEADER struct or a PNG image in its entirety i.e uncompressed.
// https://handwiki.org/wiki/ICO_(file_format)

// multibyte integers in .ico files are stored LSB first (using little endian byte order), just like .bmp files

// in summary, the binary representation of an .ico file looks like { ICONDIRENTRY, pixels, <ICONDIRENTRY, pixels> ... }

enum class FILE_TYPE : unsigned short { ICON = 1, CURSOR = 2 }; // NOLINT(performance-enum-size) deliberate decision

enum class IMAGE_TYPE : int { BITMAP, PNG }; // NOLINT(performance-enum-size) needs to be a signed integer

// look up Raymond Chen's article https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083 for reference.

// typedef struct GRPICONDIRENTRY { // wingdi
//     BYTE  bWidth;
//     BYTE  bHeight;
//     BYTE  bColorCount;
//     BYTE  bReserved;
//     WORD  wPlanes;
//     WORD  wBitCount;
//     DWORD dwBytesInRes;
//     DWORD dwImageOffset;
// }

struct ICONDIRENTRY final {
        unsigned char  width;       // width of the associated bitmap in pixels (must be in the range of 0 to 256)
        unsigned char  height;      // height of the associated bitmap in pixels (must be in the range of 0 to 256)
        unsigned char  color_count; // number of colours in the colur palette, must be 0 if the bitmap doesn't use a colour palette.
        unsigned char  __;          // reserved byte, must always be 0.
        unsigned short planes;      // specifies the colour planes (should be 0 or 1) - for ICON
        // planes specifies the horizontal coordinate of the hotspot as offset from the left, in pixels - for CURSOR
        unsigned short bit_count; // specifies pixel depth - for ICON
        // bit_count specifies the vertical coordinate of the hotspot as offset from the top, in pixels - for CURSOR
        // Windows cursors have a hotspot location, that decides one exact point that is affected by mouse events https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors
        unsigned long  size;   // size of the associated bitmap in bytes
        unsigned long  offset; // offset of the associated bitmap data, from the beginning of the .ico or .cur file
        friend class icondirectory;
};

static_assert(sizeof(ICONDIRENTRY) == 16, "");
static_assert(std::is_standard_layout<ICONDIRENTRY>::value, "");

//  typedef struct GRPICONDIR { // wingdi
//      WORD idReserved;
//      WORD idType;
//      WORD idCount;
//      GRPICONDIRENTRY idEntries[];
//  } GRPICONDIR;

struct ICONDIR final {
        unsigned short reserved;  // reserved field, must always be 0
        FILE_TYPE      type;      // specifies the type of the resources contained, values other than 1 and 2 are invalid
                                  // a given ICONDIR can store one or more of either icon or cursor type images
                                  // heterogeneous types aren't allowed inside an ICONDIR
        unsigned short count;     // number of resources (images) stored in the given .ico file
        ICONDIRENTRY*  resources; // marks the beginning of the first ICONDIRENTRY struct in the ICONDIR
        friend class icondirectory;
};

static_assert(sizeof(ICONDIR) == 16, "");
static_assert(std::is_standard_layout<ICONDIR>::value, "");

class icondirectory final { // represents an .ico file

        // clang-format off
#ifdef __TEST__
    public:
#endif
        // clang-format on

        unsigned char* buffer;      // the raw byte buffer
        unsigned long  file_size;   // file size
        unsigned long  buffer_size; // length of the buffer, may include trailing unused bytes if construction involved a buffer reuse
        unsigned short reserved;    // must be 0
        FILE_TYPE      type;
        unsigned short entry_count;        // number of ICONDIRENTRYs in the file
        std::vector<ICONDIRENTRY> entries; // entries stored in the file

        // will initialize reserved, type and entry_count members of the class
        bool parse_icondirectory(_In_reads_bytes_(size) const unsigned char* const imstream, _In_ const unsigned long size) noexcept {
            UNREFERENCED_PARAMETER(size);

            if (!imstream) {
                ::fputws(L"Error in " __FUNCTIONW__ ", the received buffer is empty!\n", stderr);
                return false;
            }

            reserved = *reinterpret_cast<const unsigned short*>(imstream);
            if (reserved) { // must be 0
                ::fputws(L"Error in " __FUNCTIONW__ ", a non-zero value encountered as idReserved!\n", stderr);
                return false;
            }

            type = static_cast<FILE_TYPE>(
                *reinterpret_cast<const unsigned short*>(imstream + 2) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            );
            if (type != FILE_TYPE::ICON && type != FILE_TYPE::CURSOR) { // cannot be anything else
                ::fputws(L"Error in " __FUNCTIONW__ ", file is found not to be of type ICON or CURSOR!\n", stderr);
                return false;
            }

            // we're 4 bytes past the beginning of the buffer now
            entry_count = *reinterpret_cast<const unsigned short*>(imstream + 4); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            // handle if the file contains more resources than MAX_ALLOWED_ICONDIRENTRIES_PER_FILE
            // if (entry_count > MAX_ALLOWED_ICONDIRENTRIES_PER_FILE) {
            //     ::fputws(L"Error in " __FUNCTIONW__ ", file contains more ICONDIRENTRYs than this class can accomodate!\n", stderr);
            //     return false;
            // }

            return true;
        }

        // it is the caller's responsibility to correctly augment the buffer such that it begins with the binary data of a ICONDIRENTRY
        static constexpr ICONDIRENTRY parse_icondirectory_entry(
            _In_count_(size) const unsigned char* const imstream, _In_ const unsigned long size
        ) noexcept {
            UNREFERENCED_PARAMETER(size);

            if (!imstream) return {};
            if (*(imstream + 3)) return {}; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) must always be 0
            ICONDIRENTRY temp {};

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            temp.width       = *imstream;
            temp.height      = *(imstream + 1);
            temp.color_count = *(imstream + 2);
            temp.__          = *(imstream + 3);

            temp.planes      = *reinterpret_cast<const unsigned short*>(imstream + 4);
            temp.bit_count   = *reinterpret_cast<const unsigned short*>(imstream + 6);
            temp.size        = *reinterpret_cast<const unsigned long*>(imstream + 8);
            temp.offset      = *reinterpret_cast<const unsigned long*>(imstream + 12);
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            return temp;
        }

    public:
        // using value_type                 = RGBQUAD; // pixel type
        // using pointer                    = value_type*;
        // using const_pointer              = const value_type*;
        // using reference                  = value_type&;
        // using const_reference            = const value_type&;
        // using iterator                   = value_type*;
        // using const_iterator             = const value_type*;
        using size_type          = unsigned long long;
        using difference_type    = long long;

        icondirectory() noexcept = default; // will be good enough

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init), intentional
        explicit icondirectory(_In_ const wchar_t* const filename) noexcept :
            buffer { internal::open(filename, file_size) }, buffer_size { file_size } {
            if (!buffer) {
                file_size = 0;
                ::fputws(L"Error inside " __FUNCTIONW__ ", object is default initialized as a fallback\n", stderr);
                return;
            }

            if (!parse_icondirectory(buffer, file_size)) {
                ::fputws(L"Error inside " __FUNCTIONW__ ", parsing failed, object is default initialized as a fallback\n", stderr);
                ::memset(this, 0U, sizeof(icondirectory)); // NOLINT(bugprone-undefined-memory-manipulation)
                return;
            }

            entries.resize(entry_count);
            unsigned long long caret { 6 };              // skip the first six bytes and jump to the first ICONDIRENTRY
            for (unsigned i = 0; i < entry_count; ++i) { // try and parse all the ICONDIRENTRYs in the file
                                                         //
                entries.at(i)  = icondirectory::parse_icondirectory_entry(buffer + caret, file_size);
                // update caret
                caret         += sizeof(ICONDIRENTRY) + entries.at(i).size;
            }
        }

        icondirectory(_In_ const icondirectory& other) noexcept { }

        icondirectory& operator=(_In_ const icondirectory& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        icondirectory(_In_ icondirectory&& other) noexcept { }

        icondirectory& operator=(_In_ icondirectory&& other) noexcept {
            if (std::addressof(other) == this) return *this;

            return *this;
        }

        ~icondirectory() noexcept {
            delete[] buffer;
            ::memset(this, 0U, sizeof(icondirectory)); // NOLINT(bugprone-undefined-memory-manipulation)
        }

        size_type image_count() const noexcept { return entry_count; }

        bool      to_file(_In_ const wchar_t* const filename) const noexcept { return internal::serialize(filename, buffer, buffer_size); }

        bitmap    image_to_bitmap(_In_opt_ const unsigned position = 0) const noexcept { }
};
