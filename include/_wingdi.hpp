#pragma once

// clang-format off
#include <internal.hpp>
// clang-format on

#ifndef  __DUMMY_WINGDI_HPP
    #define  __DUMMY_WINGDI_HPP 1
#endif

extern "C" {

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
    struct RGBQUAD final {
            unsigned char rgbBlue;
            unsigned char rgbGreen;
            unsigned char rgbRed;
            unsigned char rgbReserved;
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    struct BITMAPINFOHEADER final {
            unsigned       biSize;
            int            biWidth;
            int            biHeight;
            unsigned short biPlanes;
            unsigned short biBitCount;
            unsigned       biCompression;
            unsigned       biSizeImage;
            int            biXPelsPerMeter;
            int            biYPelsPerMeter;
            unsigned       biClrUsed;
            unsigned       biClrImportant;
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader

#pragma pack(push, 2) // this is critical here as this struct is supposed to be 14 bytes in size

    struct BITMAPFILEHEADER final {
            unsigned short bfType;
            unsigned       bfSize;
            unsigned short bfReserved1;
            unsigned short bfReserved2;
            unsigned       bfOffBits;
    };

#pragma pack(pop)

    enum class COMPRESSION : unsigned char {
        BI_RGB       = 0x1,
        BI_RLE8      = 0x2,
        BI_RLE4      = 0x3,
        BI_BITFIELDS = 0x4,
        BI_JPEG      = 0x5,
        BI_PNG       = 0x6,
        BI_CMYK      = 0xB,
        BI_CMYKRLE8  = 0xC,
        BI_CMYKRLE4  = 0xD,
    };

    struct ICONDIRENTRY final {
            unsigned char  bWidth;      // width of the associated image in pixels (must be in the range of 0 to 256)
            unsigned char  bHeight;     // height of the associated image in pixels (must be in the range of 0 to 256)
            unsigned char  bColorCount; // number of colours in the colur palette, must be 0 if the image doesn't use a colour palette
            unsigned char  bReserved;   // reserved byte, must always be 0
            unsigned short wPlanes;     // for .ico - specifies the colour planes (should be 0 or 1)
            // for .cur - specifies the horizontal coordinate of the hotspot as offset from the left, in pixels
            unsigned short wBitCount; // for .ico - specifies pixel depth
            // for .cur - specifies the vertical coordinate of the hotspot as offset from the top, in pixels
            // Windows cursors have a hotspot location that decides one exact point that is affected by mouse events https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors
            unsigned       dwBytesInRes;  // size of the associated image in bytes
            unsigned       dwImageOffset; // offset of the associated image data, from the beginning of the .ico or .cur file
    };

    struct ICONDIR final {
            unsigned short idReserved; // reserved, must always be 0
            unsigned short idType;     // specifies the type of the resources contained, values other than 1 and 2 are invalid
            // an ICONDIR can store one or more of either icon or cursor type resources, heterogeneous mixtures of icons and cursors aren't permitted
            unsigned short idCount;      // number of resources (images) stored in the given .ico file
            ICONDIRENTRY   idEntries[1]; // NOLINT(modernize-avoid-c-arrays)
    };

    struct ICONIMAGE final {
            BITMAPINFOHEADER icHeader;
            RGBQUAD          icColors[1]; // NOLINT(modernize-avoid-c-arrays)
            unsigned char    icXOR[1];    // NOLINT(modernize-avoid-c-arrays)
            unsigned char    icAND[1];    // NOLINT(modernize-avoid-c-arrays)
    };
}

#undef __INTERNAL
