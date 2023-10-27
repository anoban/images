#pragma once

#ifndef __BMP_H_
    #define __BMP_H_

    #include <array>
    #include <cassert>
    #include <cstdint>
    #include <format>
    #include <limits>
    #include <stdexcept>
    #include <string>
    #include <vector>

    #ifdef _WIN32
        #define _AMD64_ // architecture
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
    #endif

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>

namespace bmp {

    constexpr std::array<uint8_t, 2> soi {
        {'B', 'M'}
    };

    #pragma pack(push, 1)
    struct BITMAPFILEHEADER {
            std::array<uint8_t, 2> SOI {};      // 'B', 'M'
            uint32_t               FSIZE {};
            uint32_t               RESERVED {}; // this is actually two consecutive 16 bit elements, but who cares :)
            uint32_t               PIXELDATASTART {};
    };
    #pragma pack(pop)

    enum class COMPRESSIONKIND : uint32_t { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN }; // uint32_t

    #pragma pack(push, 2)
    struct BITMAPINFOHEADER {
            uint32_t        HEADERSIZE {};                                         // >= 40 bytes.
            uint32_t        WIDTH {};
            int32_t         HEIGHT {}; // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
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
    #pragma pack(pop)

    struct BITMAPCOREHEADER {
            uint32_t HEADERSIZE {};           // 12 bytes
            uint16_t WIDTH {};
            uint16_t HEIGHT {};
            uint16_t NPLANES {};              // must be 1
            uint16_t NBITSPERPIXEL {};        // 1, 4, 8 or 24
    };

    #pragma pack(push, 1)
    struct RGBQUAD {
            uint8_t BLUE {};
            uint8_t GREEN {};
            uint8_t RED {};
            uint8_t RESERVED {}; // must be 0, but seems to be 0xFF in most BMPs, yikes!
    };
    #pragma pack(pop)

    // BMP files in OS/2 use the third variant
    struct RGBTRIPLE {
            uint8_t BLUE {};
            uint8_t GREEN {};
            uint8_t RED {};
    };

    enum class BMPPIXDATAORDERING { TOPDOWN, BOTTOMUP };

    class bmp {
        private:
            size_t               size {};
            size_t               npixels {};
            BITMAPFILEHEADER     fh {};
            BITMAPINFOHEADER     infh {};
            std::vector<RGBQUAD> pixels {};

            BITMAPFILEHEADER     parse_fileheader(_In_ const std::vector<uint8_t>& imstream);
            COMPRESSIONKIND      get_compressionkind(_In_ const uint32_t cmpkind) noexcept;
            BITMAPINFOHEADER     parse_infoheader(_In_ const std::vector<uint8_t>& imstream);
            BMPPIXDATAORDERING   get_pixelorder(_In_ const BITMAPINFOHEADER& infh) noexcept;

        public:
            void serialize(_In_ const std::wstring& path);
            bmp(void) = default;
            [[nodiscard]] bmp(_In_ const std::wstring& path);

    }; // class bmp

    [[nodiscard]] std::vector<uint8_t> open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes);

} // namespace bmp

/*
static inline void print_bmp_info(_In_ const BMP* const restrict image) {
    wprintf_s(
        L"Start marker: 424D\nFile size %Lf MiBs\nPixel data start offset: %d\n",
        ((long double) image->fhead.FSIZE) / (1024 * 1024U),
        image->fhead.PIXELDATASTART
    );
    wprintf_s(
        L"BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        L"Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries:
%u\n" L"Number of important colors: %u\n", image->infhead.HEADERSIZE, image->infhead.WIDTH, image->infhead.HEIGHT, image->infhead.NPLANES,
        image->infhead.NBITSPERPIXEL,
        image->infhead.IMAGESIZE,
        image->infhead.RESPPMX,
        image->infhead.RESPPMY,
        image->infhead.NCMAPENTRIES,
        image->infhead.NIMPCOLORS
    );
    switch (image->infhead.CMPTYPE) {
        case RGB       : _putws(L"BITMAPINFOHEADER.CMPTYPE: RGB"); break;
        case RLE4      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE4"); break;
        case RLE8      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE8"); break;
        case BITFIELDS : _putws(L"BITMAPINFOHEADER.CMPTYPE: BITFIELDS"); break;
    }

    wprintf_s(L"%s BMP file\n", is_compressed(image->infhead) ? L"Compressed" : L"Uncompressed");
    wprintf_s(L"BMP pixel ordering: %s\n", get_pixel_order(image->infhead) ? L"BOTTOMUP" : L"TOPDOWN");
    return;
}

// en enum to specify the RGB -> BW conversion method.
// AVERAGE takes the mean of R, G and B values.
// WEIGHTED_AVERAGE does GREY = (0.299 R) + (0.587 G) + (0.114 B)
// LUMINOSITY does LUM = (0.2126 R) + (0.7152 G) + (0.0722 B)
typedef enum { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY } RGBTOBWKIND;

// If inplace = true, return value can be safely ignored.
static inline BMP to_blacknwhite(_In_ const BMP* image, _In_ const RGBTOBWKIND conversion_kind, _In_ const bool inplace) {
    // ___RGBQUAD encoding assumed.
    BMP local = *image;
    if (!inplace) {
        local.pixel_buffer = malloc(image->fsize - 54);
        if (!local.pixel_buffer) {
            wprintf_s(L"Error in %s (%s, %d), malloc returned nullptr\n", __FUNCTIONW__, __FILEW__, __LINE__);
            return (BMP) {
                .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                        .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                        .pixel_buffer = nullptr
            };
        }
        memcpy_s(local.pixel_buffer, local.fsize - 54, image->pixel_buffer, image->fsize - 54);
    }

    switch (conversion_kind) {
        case AVERAGE :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =
                    ((local.pixel_buffer[i].BLUE + local.pixel_buffer[i].GREEN + local.pixel_buffer[i].RED) / 3); // plain arithmetic mean
            }
            break;
        case WEIGHTED_AVERAGE :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =            // weighted average
                    (uint8_t) ((local.pixel_buffer[i].BLUE * 0.299L) + (local.pixel_buffer[i].GREEN * 0.587L) +
                               (local.pixel_buffer[i].RED * 0.114L));
            }
            break;
        case LUMINOSITY :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =
                    (uint8_t) ((local.pixel_buffer[i].BLUE * 0.2126L) + (local.pixel_buffer[i].GREEN * 0.7152L) +
                               (local.pixel_buffer[i].RED * 0.0722L));
            }
            break;
        // case BINARY :
        //     for (size_t i = 0; i < local.npixels; ++i) {
        //         local.pixel_buffer[i].BLUE  = local.pixel_buffer[i].BLUE > 128 ? 255 : 0;
        //         local.pixel_buffer[i].GREEN = local.pixel_buffer[i].GREEN > 128 ? 255 : 0;
        //         local.pixel_buffer[i].RED   = local.pixel_buffer[i].RED > 128 ? 255 : 0;
        //     }
        //     break;
        case BINARY :
            uint64_t value = 0;
            for (size_t i = 0; i < local.npixels; ++i) {
                value                      = (local.pixel_buffer[i].BLUE + local.pixel_buffer[i].GREEN + local.pixel_buffer[i].RED);
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED = (value / 3) > 128 ? 255 : 0;
            }
            break;
    }
    return local;
}

// The color palette in pixel buffer is in BGR order NOT RGB!
typedef enum { REMRED, REMGREEN, REMBLUE, REMRG, REMRB, REMGB } RMCOLOURKIND;

// If inplace = true, return value can be safely ignored.
static inline BMP remove_color(_In_ const BMP* image, _In_ const RMCOLOURKIND rmcolor, _In_ const bool inplace) {
    BMP local = *image;
    if (!inplace) {
        local.pixel_buffer = malloc(image->fsize - 54);
        if (!local.pixel_buffer) {
            wprintf_s(L"Error in %s (%s, %d), malloc returned nullptr\n", __FUNCTIONW__, __FILEW__, __LINE__);
            return (BMP) {
                .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                        .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                        .pixel_buffer = nullptr
            };
        }
        memcpy_s(local.pixel_buffer, local.fsize - 54, image->pixel_buffer, image->fsize - 54);
    }

    switch (rmcolor) {
        case REMRED :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = 0;
            break;
        case REMGREEN :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].GREEN = 0;
            break;
        case REMBLUE :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].BLUE = 0;
            break;
        case REMRG :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = local.pixel_buffer[i].GREEN = 0;
            break;
        case REMRB :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = local.pixel_buffer[i].BLUE = 0;
            break;
        case REMGB :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].GREEN = local.pixel_buffer[i].BLUE = 0;
            break;
    }
    return local;
}
*/
#endif // !__BMP_H_
