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
            std::array<uint8_t, 2> SOI {};      // BM
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

    class bmp {
        private:
            size_t               size {};
            size_t               npixels {};
            BITMAPFILEHEADER     fh {};
            BITMAPINFOHEADER     infh {};
            std::vector<RGBQUAD> pixels {};

            BITMAPFILEHEADER     parse_fileheader(_In_ const std::vector<uint8_t>& imstream);
            BITMAPINFOHEADER     parse_infoheader(_In_ const std::vector<uint8_t>& imstream);

    }; // class bmp

    std::vector<uint8_t> open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes);
}

static __forceinline COMPRESSIONKIND __stdcall get_bmp_compression_kind(_In_ const uint32_t cmpkind) {
    switch (cmpkind) {
        case 0 : return RGB;
        case 1 : return RLE8;
        case 2 : return RLE4;
        case 3 : return BITFIELDS;
    }
    return -1;
}

typedef enum { TOPDOWN, BOTTOMUP } BMPPIXDATAORDERING;

static __forceinline BMPPIXDATAORDERING __stdcall get_pixel_order(_In_ const __BITMAPINFOHEADER bmpinfh) {
    if (bmpinfh.HEIGHT >= 0) return BOTTOMUP;
    return TOPDOWN;
}

static inline BMP new_bmp(_In_ wchar_t* file_name) {
    BMP image = {
        .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                .pixel_buffer = nullptr
    };

    uint8_t* buffer = open_image(file_name, &image.fsize);
    if (!buffer) {
        // open_image will print the error messages, so no need to do that here.
        wprintf_s(L"Error in %s (%s, %d), open_image returned nullptr\n", __FUNCTIONW__, __FILEW__, __LINE__);
        return (BMP) {
            .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                    .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                    .pixel_buffer = nullptr
        };
    }

    image.fhead   = parse_bitmapfile_header(buffer, image.fsize);
    image.infhead = parse_bitmapinfo_header(buffer, image.fsize);
    assert(!((image.fsize - 54) % 4)); // Make sure that the number of bytes in the pixel buffer is divisible by 4 without remainders.

    image.npixels      = (image.fsize - 54) / 4;
    image.pixel_buffer = malloc(image.fsize - 54);
    if (!image.pixel_buffer) {         // If malloc failed,
        wprintf_s(L"Error in %s (%s, %d), malloc returned nullptr\n", __FUNCTIONW__, __FILEW__, __LINE__);
        free(buffer);
        return (BMP) {
            .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                    .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                    .pixel_buffer = nullptr
        };
    }

    // memcyp_s will throw an invalid parameter exception when it finds that the destination size is less than source size!
    // be careful.
    memcpy_s((uint8_t*) image.pixel_buffer, image.fsize - 54, buffer + 54, image.fsize - 54);
    free(buffer);
    return image; // caller needs to free the image.pixel_buffer
}

static inline bool serialize_bmp(_In_ const BMP* const restrict image, _In_ const wchar_t* restrict file_name) {
    HANDLE hfile = CreateFileW(file_name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return false;
    }

    uint32_t nbyteswritten = 0;
    uint8_t  tmp[54]       = { 0 };
    memcpy_s(tmp, 54, &image->fhead, 14);
    memcpy_s(tmp + 14, 40, &image->infhead, 40);

    if (!WriteFile(hfile, tmp, 54, &nbyteswritten, nullptr)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile\n", GetLastError());
        CloseHandle(hfile);
        return false;
    }
    CloseHandle(hfile);

    hfile = CreateFileW(file_name, FILE_APPEND_DATA, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return false;
    }
    if (!WriteFile(hfile, image->pixel_buffer, image->fsize - 54, &nbyteswritten, nullptr)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile\n", GetLastError());
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}

static inline void print_bmp_info(_In_ const BMP* const restrict image) {
    wprintf_s(
        L"Start marker: 424D\nFile size %Lf MiBs\nPixel data start offset: %d\n",
        ((long double) image->fhead.FSIZE) / (1024 * 1024U),
        image->fhead.PIXELDATASTART
    );
    wprintf_s(
        L"BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        L"Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries: %u\n"
        L"Number of important colors: %u\n",
        image->infhead.HEADERSIZE,
        image->infhead.WIDTH,
        image->infhead.HEIGHT,
        image->infhead.NPLANES,
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

} // namespace bmp

#endif // !__BMP_H_