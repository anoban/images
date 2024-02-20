#pragma once
#ifndef __BMP_H_
    #define __BMP_H_

    #include <assert.h>
    #include <limits.h>
    #include <stdint.h>

    #define _AMD64_ // architecture
    #define WIN32_LEAN_AND_MEAN
    #define WIN32_EXTRA_MEAN

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>

static const uint8_t SOIMAGE[2] = { 'B', 'M' };

// every Windows BMP begins with a BITMAPFILEHEADER struct
// this helps in recognizing the file format as .bmp
// the first two bytes will be 'B', 'M'

struct BITMAPFILEHEADER {
        uint8_t  SOI[2];   // 'B', 'M'
        uint32_t FSIZE;
        uint32_t RESERVED; // this is actually two consecutive 16 bit elements, but who cares :)
        uint32_t PIXELDATASTART;
};

// types of compressions used in BMP files.
enum COMPRESSIONKIND { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN };

// image header comes in two variants!
// one representing OS/2 BMP format (BITMAPCOREHEADER) and another representing the most common Windows BMP format.
// (BITMAPINFOHEADER)
// however there are no markers to identify the type of the image header present in the bmp image.
// the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct.
// sizeof BITMAPCOREHEADER is 12 bytes
// sizeof BITMAPINFOHEADER is >= 40 bytes.
// from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes!

struct BITMAPINFOHEADER {
        uint32_t        HEADERSIZE {}; // >= 40 bytes.
        uint32_t        WIDTH {};
        int32_t         HEIGHT {};     // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
        // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
        uint16_t        NPLANES {};       // must be 1
        uint16_t        NBITSPERPIXEL {}; // 1, 4, 8, 16, 24 or 32
        COMPRESSIONKIND CMPTYPE {};
        uint32_t        IMAGESIZE {};     // 0 if not compressed.
        uint32_t        RESPPMX {};       // resolution in pixels per meter along x axis.
        uint32_t        RESPPMY {};       // resolution in pixels per meter along y axis.
        uint32_t        NCMAPENTRIES {};  // number of entries in the colourmap that are used.
        uint32_t        NIMPCOLORS {};    // number of important colors.
};

// a BMP with BITMAPCOREHEADER cannot be compressed.
// and is very rarely used in modern BMPs.
struct BITMAPCOREHEADER {
        uint32_t HEADERSIZE {};    // 12 bytes
        uint16_t WIDTH {};
        uint16_t HEIGHT {};
        uint16_t NPLANES {};       // must be 1
        uint16_t NBITSPERPIXEL {}; // 1, 4, 8 or 24
};

    #pragma pack(push, 1)
struct RGBQUAD {
        uint8_t BLUE {};
        uint8_t GREEN {};
        uint8_t RED {};
        uint8_t RESERVED { 0xFF }; // must be 0, but seems to be 0xFF in most BMPs.
};
    #pragma pack(pop)

// BMP files in OS/2 use this variant.
struct RGBTRIPLE {
        uint8_t BLUE {};
        uint8_t GREEN {};
        uint8_t RED {};
};

enum class BMPPIXDATAORDERING { TOPDOWN, BOTTOMUP };

enum class TOBWKIND : uint32_t { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY };

enum class RGBCOMB : uint32_t { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE };

typedef struct bmp {
        size_t               size {};
        size_t               npixels {};
        BITMAPFILEHEADER     fh {};
        BITMAPINFOHEADER     infh {};
        std::vector<RGBQUAD> pixels {};

        BITMAPFILEHEADER     parse_fileheader(_In_ const std::vector<uint8_t>& imstream);

        COMPRESSIONKIND      get_compressionkind(_In_ const uint32_t cmpkind) noexcept;

        BITMAPINFOHEADER     parse_infoheader(_In_ const std::vector<uint8_t>& imstream);

        BMPPIXDATAORDERING   get_pixelorder(_In_ const BITMAPINFOHEADER& header) noexcept;

        bmp(void) = default;
        bmp(_In_ const std::wstring& path);

        constexpr bmp(
            _In_ const BITMAPFILEHEADER& headf, _In_ const BITMAPINFOHEADER& headinf, _In_ const std::vector<RGBQUAD>& pbuff
        ) noexcept;

        void               serialize(_In_ const std::wstring& path);

        void               info(void) noexcept;

        std::optional<bmp> tobwhite(_In_ const TOBWKIND cnvrsnkind, _In_opt_ const bool inplace = false) noexcept;

        std::optional<bmp> tonegative(_In_opt_ const bool inplace = false) noexcept;

        std::optional<bmp> remove_clr(_In_ const RGBCOMB kind, _In_opt_ const bool inplace = false) noexcept;

        static bmp         gengradient(_In_opt_ const size_t heightpx = 1080, _In_opt_ const size_t widthpx = 1080);

} bmp_t; // class bmp

std::vector<uint8_t>      open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes);
// expects argv to be passed as fnames (without any preprocessing) i.e do not remove the first element in argv.

std::vector<std::wstring> remove_ext(
    _In_count_(size) wchar_t* const fnames[], _In_ const size_t length, _In_ const wchar_t* const extension
) noexcept;

#endif // !__BMP_H_
