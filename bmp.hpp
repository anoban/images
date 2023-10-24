#pragma once
#ifndef __BMP_H__
    #define __BMP_H__

    #include <array>
    #include <cassert>
    #include <cstdint>
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

    class bmp {
    // every Windows BMP begins with a BITMAPFILEHEADER struct
    #pragma pack(push, 1)
            struct BITMAPFILEHEADER {
                    std::array<uint8_t, 2> soi {};         // 'B' 'M'
                    uint32_t               fsize {};       // size of the .BMP file
                    uint32_t               reserved {};    // this is actually two consecutive 16 bit elements, but who cares :)
                    uint32_t               pixeloffset {}; // offset where the pixels start in the .BMP file.
            };
    #pragma pack(pop)

            // types of compressions used in BMP files.
            enum class BMPCOMPRESSIONKIND : uint32_t { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN };

    #pragma pack(push, 2)
            struct BITMAPINFOHEADER {
                    uint32_t hsize;  // must be >= 40 bytes.
                    uint32_t width;
                    int32_t  height; // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
                    // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
                    uint16_t planes;                // number of planes, must be 1
                    uint16_t bitsperpixel;          // 1, 4, 8, 16, 24 or 32
                    BMPCOMPRESSIONKIND cmpkind;
                    uint32_t           imgsize;     // 0 if not compressed.
                    uint32_t           res_x;       // resolution in pixels per meter along x axis.
                    uint32_t           res_y;       // resolution in pixels per meter along y axis.
                    uint32_t           cmapentries; // number of entries in the colourmap that are used.
                    uint32_t           impcolors;   // number of important colors.
            };
    #pragma pack(pop)

            // a BMP with BITMAPCOREHEADER cannot be compressed, rarely used in modern .BMP files.
            struct BITMAPCOREHEADER {
                    uint32_t hsize;        // 12 bytes
                    uint16_t width;
                    uint16_t height;
                    uint16_t planes;       // must be 1
                    uint16_t bitsperpixel; // 1, 4, 8 or 24
            };

    #pragma pack(push, 1)
            // most commonly used BMP pixel type.
            struct RGBQUAD {
                    uint8_t blue;
                    uint8_t green;
                    uint8_t red;
                    uint8_t reserved; // must be 0, but seems to be 0xFF in most BMPs, yikes!
            };
    #pragma pack(pop)

            // BMP files in OS/2 use the third variant, rarely used in modern .BMP files.
            struct RGBTRIPLE {
                    uint8_t blue;
                    uint8_t green;
                    uint8_t red;
            };

            // helper routines.
            static inline BITMAPFILEHEADER parse_fileheader(_In_ const uint8_t* imstream, _In_ const uint64_t fsize) {
                static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER is not 14 bytes in size.");
                assert(fsize >= sizeof(BITMAPFILEHEADER));

                BITMAPFILEHEADER header = { 0, 0, 0, 0 };
                // due to little endianness, two serial bytes 0x42, 0x4D will be interpreted as 0x4D42 when casted as
                // an uint16_t yikes!, thereby warranting a little bitshift.
                header.SOI              = (((uint16_t) (*(imstream + 1))) << 8) | ((uint16_t) (*imstream));
                if (header.SOI != SOBMP) {
                    fputws(L"Error in parse_bitmapfile_header, file appears not to be a Windows BMP file\n", stderr);
                    return header;
                }
                header.FSIZE          = *((uint32_t*) (imstream + 2));
                header.PIXELDATASTART = *((uint32_t*) (imstream + 10));
                return header;
            }

            static inline BITMAPINFOHEADER parse_infoheader(_In_ const uint8_t* const imstream, _In_ const uint64_t fsize) {
                static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "Error: __BITMAPINFOHEADER is not 40 bytes in size");
                assert(fsize >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

                BITMAPINFOHEADER header = { 0, 0, 0, 0, 0, BMPCOMPRESSIONKIND::UNKNOWN, 0, 0, 0, 0, 0 };
                if (*((uint32_t*) (imstream + 14U)) > 40) {
                    fputws(L"BITMAPINFOHEADER larger than 40 bytes! BMP image seems to contain an unparsable file info header", stderr);
                    return header;
                }
                header.HEADERSIZE    = *((uint32_t*) (imstream + 14U));
                header.WIDTH         = *((uint32_t*) (imstream + 18U));
                header.HEIGHT        = *((uint32_t*) (imstream + 22U));
                header.NPLANES       = *((uint16_t*) (imstream + 26U));
                header.NBITSPERPIXEL = *((uint16_t*) (imstream + 28U));
                header.CMPTYPE       = get_bmp_compression_kind(*((uint32_t*) (imstream + 30U)));
                header.IMAGESIZE     = *((uint32_t*) (imstream + 34U));
                header.RESPPMX       = *((uint32_t*) (imstream + 38U));
                header.RESPPMY       = *((uint32_t*) (imstream + 42U));
                header.NCMAPENTRIES  = *((uint32_t*) (imstream + 46U));
                header.NIMPCOLORS    = *((uint32_t*) (imstream + 50U));
                return header;
            }

            // class data.

        private:
            size_t               fsize {};   // size of the .BMP file
            size_t               npixels {}; // number of pixels in the pixel buffer.
            BITMAPFILEHEADER     fheader {};
            BITMAPINFOHEADER     infheader {};
            std::vector<RGBQUAD> pixels {};  // pixel buffer.

        public:
            [[nodiscard]] inline bmp(_In_ const std::wstring& file_path) { }
            [[nodiscard]] inline bmp() = default;

            // a static method like bmp::serialize(_In_ const bmp& image) might be cool, but will likely lead to ugly syntax when method
            // chaining is involved, so NO.
            inline void serialize(void) { }
    };

} // namespace bmp

#endif // !__BMP_H__